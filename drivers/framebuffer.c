#include "framebuffer.h"
#include "io.h"

#define FB_COMMAND_PORT 0x3D4
#define FB_DATA_PORT    0x3D5
#define FB_HIGH_BYTE_COMMAND 14
#define FB_LOW_BYTE_COMMAND  15
#define SERIAL_PORT 0x3F8

char *fb = (char *) 0x000B8000;
static unsigned short cursor_x = 0;
static unsigned short cursor_y = 0;
static unsigned char current_fg = FB_WHITE;
static unsigned char current_bg = FB_BLACK;

void fb_move_cursor(unsigned short pos) {
    outb(FB_COMMAND_PORT, FB_HIGH_BYTE_COMMAND);
    outb(FB_DATA_PORT, ((pos >> 8) & 0x00FF));
    outb(FB_COMMAND_PORT, FB_LOW_BYTE_COMMAND);
    outb(FB_DATA_PORT, pos & 0x00FF);
}

void fb_write_cell(unsigned int i, char c, unsigned char fg, unsigned char bg) {
    fb[i] = c;
    fb[i + 1] = ((bg & 0x0F) << 4) | (fg & 0x0F);
}

void fb_put_char(char c) {
    outb(SERIAL_PORT, c);

    if (c == '\n') {
        cursor_x = 0;
        cursor_y++;
        outb(SERIAL_PORT, '\r');
    } else if (c == '\b') {
        fb_backspace();
    } else {
        unsigned int i = (cursor_y * 80 + cursor_x) * 2;
        fb_write_cell(i, c, current_fg, current_bg);
        cursor_x++;
    }

    if (cursor_x >= 80) {
        cursor_x = 0;
        cursor_y++;
    }

    if (cursor_y >= 25) {
        for (int y = 1; y < 25; y++) {
            for (int x = 0; x < 80; x++) {
                unsigned int dest = ((y - 1) * 80 + x) * 2;
                unsigned int src = (y * 80 + x) * 2;
                fb[dest] = fb[src];
                fb[dest + 1] = fb[src + 1];
            }
        }
        for (int x = 0; x < 80; x++) {
            unsigned int i = (24 * 80 + x) * 2;
            fb_write_cell(i, ' ', current_fg, current_bg);
        }
        cursor_y = 24;
        cursor_x = 0;
    }

    fb_move_cursor(cursor_y * 80 + cursor_x);
}

void fb_backspace(void) {
    if (cursor_x > 0) {
        cursor_x--;
    } else if (cursor_y > 0) {
        cursor_y--;
        cursor_x = 79;
    }
    unsigned int i = (cursor_y * 80 + cursor_x) * 2;
    fb_write_cell(i, ' ', current_fg, current_bg);
    fb_move_cursor(cursor_y * 80 + cursor_x);
    outb(SERIAL_PORT, '\b');
}

void fb_print(const char *str) {
    while (*str) {
        fb_put_char(*str);
        str++;
    }
}

void fb_clear(void) {
    for (int i = 0; i < 80 * 25 * 2; i += 2) {
        fb_write_cell(i, ' ', current_fg, current_bg);
    }
    cursor_x = 0;
    cursor_y = 0;
    fb_move_cursor(0);
}

void fb_move(unsigned short x, unsigned short y) {
    cursor_x = x;
    cursor_y = y;
    fb_move_cursor(cursor_y * 80 + cursor_x);
}

void fb_set_color(unsigned char fg, unsigned char bg) {
    current_fg = fg;
    current_bg = bg;
}
