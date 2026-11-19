#include "interrupts.h"
#include "pic.h"
#include "io.h"
#include "framebuffer.h"
#include "keyboard.h"

#define INTERRUPTS_DESCRIPTOR_COUNT 256
#define INTERRUPTS_KEYBOARD         33

struct IDTDescriptor idt_descriptors[INTERRUPTS_DESCRIPTOR_COUNT];
struct IDT idt;

void interrupts_init_descriptor(s32int index, u32int address) {
    idt_descriptors[index].offset_high      = (address >> 16) & 0xFFFF;
    idt_descriptors[index].offset_low       = (address & 0xFFFF);
    idt_descriptors[index].segment_selector = 0x08;
    idt_descriptors[index].reserved         = 0x00;
    idt_descriptors[index].type_and_attr    = (0x01 << 7) | (0x00 << 6) | (0x00 << 5) | 0x0E;
}

void interrupts_install_idt(void) {
    interrupts_init_descriptor(INTERRUPTS_KEYBOARD, (u32int) interrupt_handler_33);

    idt.address = (u32int) &idt_descriptors;
    idt.size    = sizeof(struct IDTDescriptor) * INTERRUPTS_DESCRIPTOR_COUNT - 1;
    load_idt((u32int) &idt);

    pic_remap(PIC_1_OFFSET, PIC_2_OFFSET);
    outb(0x21, inb(0x21) & ~(1 << 1));
}

void interrupt_handler(__attribute__((unused)) struct cpu_state cpu,
                       u32int interrupt,
                       __attribute__((unused)) struct stack_state stack) {
    u8int input;
    u8int ascii;

    switch (interrupt) {
        case INTERRUPTS_KEYBOARD:
            while (inb(0x64) & 1) {
                input = keyboard_read_scan_code();
                if (!(input & 0x80)) {
                    if (input <= KEYBOARD_MAX_ASCII) {
                        ascii = keyboard_scan_code_to_ascii(input);
                        if (ascii != 0) {
                            if (ascii == '\b') {
                                fb_backspace();
                            } else if (ascii == '\n') {
                                fb_put_char('\n');
                            } else {
                                fb_put_char(ascii);
                            }
                            keyboard_buffer_put(ascii);
                        }
                    }
                }
            }
            pic_acknowledge(interrupt);
            break;

        default:
            break;
    }
}
