#include "interrupts.h"
#include "pic.h"
#include "io.h"
#include "framebuffer.h"
#include "keyboard.h"

#define INTERRUPTS_DESCRIPTOR_COUNT 256
#define INTERRUPTS_KEYBOARD         33  /* IRQ1 = PIC_1_OFFSET + 1 */
#define INTERRUPTS_COM1             36  /* IRQ4 = PIC_1_OFFSET + 4 */

struct IDTDescriptor idt_descriptors[INTERRUPTS_DESCRIPTOR_COUNT];
struct IDT idt;

/* number of characters typed on the current input line; stops backspace
   from deleting the prompt and stops the line growing past what
   readline() can store */
static u32int line_length = 0;

void interrupts_init_descriptor(s32int index, u32int address) {
    idt_descriptors[index].offset_high      = (address >> 16) & 0xFFFF;
    idt_descriptors[index].offset_low       = (address & 0xFFFF);
    idt_descriptors[index].segment_selector = 0x08;
    idt_descriptors[index].reserved         = 0x00;
    idt_descriptors[index].type_and_attr    = (0x01 << 7) | (0x00 << 6) | (0x00 << 5) | 0x0E;
}

void interrupts_install_idt(void) {
    interrupts_init_descriptor(INTERRUPTS_KEYBOARD, (u32int) interrupt_handler_33);
    interrupts_init_descriptor(INTERRUPTS_COM1,     (u32int) interrupt_handler_36);

    idt.address = (u32int) &idt_descriptors;
    idt.size    = sizeof(struct IDTDescriptor) * INTERRUPTS_DESCRIPTOR_COUNT - 1;
    load_idt((u32int) &idt);

    pic_remap(PIC_1_OFFSET, PIC_2_OFFSET);

    /* pic_remap masks every IRQ: unmask IRQ1 (keyboard) and IRQ4 (COM1) */
    outb(0x21, inb(0x21) & ~((1 << 1) | (1 << 4)));
}

/* Show one input character on screen and store it in the input buffer. */
static void process_input_char(u8int ascii) {
    // We have detected a backspace
    if (ascii == '\b') {
        if (line_length > 0) {
            // Remove the last character
            line_length--;
            fb_backspace();
            keyboard_buffer_put('\b');
        }
    }
    // We have detected a newline
    else if (ascii == '\n') {
        // Move our position to a newline
        line_length = 0;
        fb_newline();
        keyboard_buffer_put('\n');
    }
    // We have detected a regular character
    else if (ascii >= 32 && ascii < 127) {
        if (line_length < KEYBOARD_LINE_MAX) {
            // Add the new character to the display
            line_length++;
            fb_put_char(ascii);
            keyboard_buffer_put(ascii);
        }
    }
}

/* Interrupt handlers ********************************************************/

void interrupt_handler(__attribute__((unused)) struct cpu_state cpu,
                       u32int interrupt,
                       __attribute__((unused)) struct stack_state stack) {
    u8int input;
    u8int ascii;

    switch (interrupt) {
        case INTERRUPTS_KEYBOARD:
            while ((inb(0x64) & 1)) {
                // 1. Read the scan code from the keyboard
                input = keyboard_read_scan_code();

                // Only process if it's not a break (key release) code
                if (!(input & 0x80)) {
                    if (input <= KEYBOARD_MAX_ASCII) {
                        // 2. Convert it to ASCII
                        ascii = keyboard_scan_code_to_ascii(input);
                        if (ascii != 0) {
                            // 3. Backspace / enter / regular character
                            process_input_char(ascii);
                        }
                    }
                }
            }
            pic_acknowledge(interrupt);
            break;

        case INTERRUPTS_COM1:
            /* -nographic mode: the terminal sends bytes over COM1 */
            while (serial_received()) {
                ascii = serial_read_byte();
                if (ascii == '\r') {
                    ascii = '\n';           /* terminals send CR for enter */
                } else if (ascii == 127) {
                    ascii = '\b';           /* terminals send DEL for backspace */
                }
                process_input_char(ascii);
            }
            pic_acknowledge(interrupt);
            break;

        default:
            break;
    }
}
