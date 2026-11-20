#include "keyboard.h"
#include "io.h"
#include "framebuffer.h"
#include "hardware_interrupt_enabler.h"

#define KEYBOARD_DATA_PORT 0x60

#define SERIAL_PORT             0x3F8
#define SERIAL_LINE_STATUS_PORT (SERIAL_PORT + 5)

/* ======================================================================
 * Task 1 - reading and converting scan codes
 * ====================================================================== */

/** keyboard_read_scan_code:
 *  Reads a scan code from the keyboard
 *
 *  @return The scan code (NOT an ASCII character!)
 */
u8int keyboard_read_scan_code(void)
{
    return inb(KEYBOARD_DATA_PORT);
}

u8int keyboard_scan_code_to_ascii(u8int scan_code)
{
    // Ignore key releases (scan codes with bit 7 set)
    if (scan_code & 0x80) {
        return 0;
    }

    // Scan code to ASCII mapping for standard US QWERTY keyboard
    switch (scan_code) {
        // Numbers row
        case 0x02: return '1';
        case 0x03: return '2';
        case 0x04: return '3';
        case 0x05: return '4';
        case 0x06: return '5';
        case 0x07: return '6';
        case 0x08: return '7';
        case 0x09: return '8';
        case 0x0A: return '9';
        case 0x0B: return '0';
        case 0x0C: return '-';
        case 0x0D: return '=';
        case 0x0E: return '\b'; // Backspace

        // Top letter row
        case 0x10: return 'q';
        case 0x11: return 'w';
        case 0x12: return 'e';
        case 0x13: return 'r';
        case 0x14: return 't';
        case 0x15: return 'y';
        case 0x16: return 'u';
        case 0x17: return 'i';
        case 0x18: return 'o';
        case 0x19: return 'p';
        case 0x1A: return '[';
        case 0x1B: return ']';
        case 0x1C: return '\n'; // Enter

        // Middle letter row
        case 0x1E: return 'a';
        case 0x1F: return 's';
        case 0x20: return 'd';
        case 0x21: return 'f';
        case 0x22: return 'g';
        case 0x23: return 'h';
        case 0x24: return 'j';
        case 0x25: return 'k';
        case 0x26: return 'l';
        case 0x27: return ';';
        case 0x28: return '\'';
        case 0x29: return '`';

        // Bottom letter row
        case 0x2B: return '\\';
        case 0x2C: return 'z';
        case 0x2D: return 'x';
        case 0x2E: return 'c';
        case 0x2F: return 'v';
        case 0x30: return 'b';
        case 0x31: return 'n';
        case 0x32: return 'm';
        case 0x33: return ',';
        case 0x34: return '.';
        case 0x35: return '/';
        case 0x39: return ' '; // Space bar

        // Numpad
        case 0x37: return '*'; // Numpad *
        case 0x47: return '7';
        case 0x48: return '8';
        case 0x49: return '9';
        case 0x4A: return '-';
        case 0x4B: return '4';
        case 0x4C: return '5';
        case 0x4D: return '6';
        case 0x4E: return '+';
        case 0x4F: return '1';
        case 0x50: return '2';
        case 0x51: return '3';
        case 0x52: return '0';
        case 0x53: return '.';

        default: return 0; // Unknown scan code
    }
}

/* ======================================================================
 * Task 2 - input buffer API
 *
 * A circular buffer shared between the interrupt handler (producer) and
 * getc() (consumer). head is only written by the interrupt handler, tail
 * is only written by getc(). One slot is always left unused, so:
 *     head == tail            -> buffer empty
 *     (head + 1) % SIZE == tail -> buffer full
 * Both indices are volatile because they change inside an interrupt.
 * ====================================================================== */
#define INPUT_BUFFER_SIZE 256

static volatile u8int input_buffer[INPUT_BUFFER_SIZE];
static volatile u32int buffer_head = 0; /* next slot to write */
static volatile u32int buffer_tail = 0; /* next slot to read  */

/* Called from the interrupt handler. If the buffer is full the new
   character is dropped (the oldest unread input is kept). */
void keyboard_buffer_put(u8int c)
{
    u32int next = (buffer_head + 1) % INPUT_BUFFER_SIZE;
    if (next == buffer_tail) {
        return; /* overflow: drop the character */
    }
    input_buffer[buffer_head] = c;
    buffer_head = next;
}

/* Remove and return one character from the buffer. If the buffer is
   empty the CPU is halted until the next interrupt arrives, instead of
   spinning in a busy loop. */
u8int getc(void)
{
    while (1) {
        /* interrupts off while we look at / update the buffer, so the
           handler cannot change it half way through */
        disable_hardware_interrupts();
        if (buffer_tail != buffer_head) {
            u8int c = input_buffer[buffer_tail];
            buffer_tail = (buffer_tail + 1) % INPUT_BUFFER_SIZE;
            enable_hardware_interrupts();
            return c;
        }
        /* buffer empty: "sti; hlt" re-enables interrupts and halts in one
           step (sti takes effect after the next instruction), so a key
           press cannot arrive between the check above and the hlt */
        __asm__ volatile("sti; hlt");
    }
}

/* Read characters until newline into buf (max_len includes the '\0').
   The characters were already shown on screen by the interrupt handler,
   so readline only has to store them. Returns the length of the line. */
int readline(char *buf, int max_len)
{
    int i = 0;

    if (buf == 0 || max_len <= 0) {
        return 0;
    }

    while (1) {
        u8int c = getc();

        if (c == '\n') {
            break;
        } else if (c == '\b') {
            if (i > 0) {
                i--;
            }
        } else if (i < max_len - 1) {
            buf[i++] = c;
        }
        /* else: line is full, extra characters are ignored */
    }

    buf[i] = '\0';
    return i;
}

/* ======================================================================
 * COM1 serial port
 *
 * With "make run" (-nographic) there is no VGA window and no PS/2
 * keyboard: the terminal is connected to COM1 instead. Received bytes
 * raise IRQ4, and the interrupt handler feeds them through the same
 * path as keyboard input, so getc()/readline() work in both modes.
 * ====================================================================== */
void init_serial(void)
{
    outb(SERIAL_PORT + 1, 0x00);    /* disable UART interrupts while configuring */
    outb(SERIAL_PORT + 3, 0x80);    /* enable DLAB (set baud rate divisor) */
    outb(SERIAL_PORT + 0, 0x03);    /* divisor 3 (lo byte) -> 38400 baud */
    outb(SERIAL_PORT + 1, 0x00);    /* divisor hi byte */
    outb(SERIAL_PORT + 3, 0x03);    /* 8 bits, no parity, one stop bit */
    outb(SERIAL_PORT + 2, 0xC7);    /* enable FIFO, clear it, 14-byte threshold */
    outb(SERIAL_PORT + 4, 0x0B);    /* DTR, RTS and OUT2 (OUT2 connects the UART IRQ to the PIC) */
    outb(SERIAL_PORT + 1, 0x01);    /* interrupt when a byte is received */
}

int serial_received(void)
{
    return inb(SERIAL_LINE_STATUS_PORT) & 1;
}

u8int serial_read_byte(void)
{
    return inb(SERIAL_PORT);
}
