#include "keyboard.h"
#include "io.h"
#include "framebuffer.h"

#define SERIAL_PORT 0x3F8
#define SERIAL_LINE_STATUS_PORT (SERIAL_PORT + 5)

void init_serial(void) {
    outb(SERIAL_PORT + 1, 0x00);    // Disable interrupts
    outb(SERIAL_PORT + 3, 0x80);    // Enable DLAB (set baud rate divisor)
    outb(SERIAL_PORT + 0, 0x03);    // Set divisor to 3 (lo byte) -> 38400 baud
    outb(SERIAL_PORT + 1, 0x00);    // High byte
    outb(SERIAL_PORT + 3, 0x03);    // 8 bits, no parity, one stop bit
    outb(SERIAL_PORT + 2, 0xC7);    // Enable FIFO, clear transmit/receive FIFO, 14-byte threshold
    outb(SERIAL_PORT + 4, 0x0B);    // IRQs enabled, RTS/DSR set
}

int serial_received(void) {
    return inb(SERIAL_LINE_STATUS_PORT) & 1;
}

char serial_read(void) {
    while (!serial_received());
    return inb(SERIAL_PORT);
}

u8int getc(void) {
    u8int c = serial_read();
    if (c == '\r') {
        c = '\n';
    }
    return c;
}

int readline(char *buf, int max_len) {
    int i = 0;
    while (i < max_len - 1) {
        u8int c = getc();
        
        if (c == '\n') {
            buf[i] = '\0';
            fb_print("\n");
            return i;
        } else if (c == '\b' || c == 127) {
            if (i > 0) {
                i--;
                fb_backspace();
            }
        } else {
            buf[i++] = c;
            char s[2] = {c, '\0'};
            fb_print(s);
        }
    }
    buf[i] = '\0';
    return i;
}

u8int keyboard_read_scan_code(void) {
    return 0;
}

void keyboard_buffer_put(__attribute__((unused)) u8int c) {
}

u8int keyboard_scan_code_to_ascii(__attribute__((unused)) u8int scan_code) {
    return 0;
}
