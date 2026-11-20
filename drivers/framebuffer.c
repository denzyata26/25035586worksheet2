#include "framebuffer.h"
#include "io.h"

#define FB_ADDRESS           0x000B8000
#define FB_COMMAND_PORT      0x3D4
#define FB_DATA_PORT         0x3D5
#define FB_HIGH_BYTE_COMMAND 14
#define FB_LOW_BYTE_COMMAND  15

/* Everything printed is also copied to COM1, so the output can be seen
   when qemu runs with -nographic (no VGA screen). */
#define SERIAL_PORT 0x3F8

static char *fb = (char *) FB_ADDRESS;
static unsigned short cursor_x = 0;
static unsigned short cursor_y = 0;
static unsigned char current_fg = FB_WHITE;
static unsigned char current_bg = FB_BLACK;

/* cell = index of the character cell (0 .. 80*25-1); each cell is 2 bytes:
   the ASCII character, then the colour byte (background << 4 | foreground) */
void fb_write_cell(unsigned int cell, char c, unsigned char fg, unsigned char bg) {
    fb[cell * 2] = c;
    fb[cell * 2 + 1] = ((bg & 0x0F) << 4) | (fg & 0x0F);
}

void fb_move_cursor(unsigned short pos) {
    outb(FB_COMMAND_PORT, FB_HIGH_BYTE_COMMAND);
    outb(FB_DATA_PORT, ((pos >> 8) & 0x00FF));
    outb(FB_COMMAND_PORT, FB_LOW_BYTE_COMMAND);
    outb(FB_DATA_PORT, pos & 0x00FF);
}

static void fb_update_cursor(void) {
    fb_move_cursor(cursor_y * FB_WIDTH + cursor_x);
}

/* move every row up by one and blank the last row */
static void fb_scroll(void) {
    for (int i = 0; i < (FB_HEIGHT - 1) * FB_WIDTH * 2; i++) {
        fb[i] = fb[i + FB_WIDTH * 2];
    }
    for (int x = 0; x < FB_WIDTH; x++) {
        fb_write_cell((FB_HEIGHT - 1) * FB_WIDTH + x, ' ', current_fg, current_bg);
    }
    cursor_y = FB_HEIGHT - 1;
}

void fb_move(unsigned short x, unsigned short y) {
    if (x >= FB_WIDTH)  x = FB_WIDTH - 1;
    if (y >= FB_HEIGHT) y = FB_HEIGHT - 1;
    cursor_x = x;
    cursor_y = y;
    fb_update_cursor();
}

void fb_set_color(unsigned char fg, unsigned char bg) {
    current_fg = fg;
    current_bg = bg;
}

void fb_clear(void) {
    for (int cell = 0; cell < FB_WIDTH * FB_HEIGHT; cell++) {
        fb_write_cell(cell, ' ', current_fg, current_bg);
    }
    fb_move(0, 0);
}

void fb_newline(void) {
    outb(SERIAL_PORT, '\r');
    outb(SERIAL_PORT, '\n');
    cursor_x = 0;
    cursor_y++;
    if (cursor_y >= FB_HEIGHT) {
        fb_scroll();
    }
    fb_update_cursor();
}

void fb_put_char(char c) {
    if (c == '\n') {
        fb_newline();
        return;
    }
    if (c == '\b') {
        fb_backspace();
        return;
    }

    outb(SERIAL_PORT, c);
    fb_write_cell(cursor_y * FB_WIDTH + cursor_x, c, current_fg, current_bg);
    cursor_x++;
    if (cursor_x >= FB_WIDTH) {
        fb_newline();
        return;
    }
    fb_update_cursor();
}

void fb_backspace(void) {
    if (cursor_x > 0) {
        cursor_x--;
    } else if (cursor_y > 0) {
        cursor_y--;
        cursor_x = FB_WIDTH - 1;
    } else {
        return;
    }
    fb_write_cell(cursor_y * FB_WIDTH + cursor_x, ' ', current_fg, current_bg);
    fb_update_cursor();

    /* serial terminal: move back, overwrite with a space, move back again */
    outb(SERIAL_PORT, '\b');
    outb(SERIAL_PORT, ' ');
    outb(SERIAL_PORT, '\b');
}

void fb_print(const char *str) {
    while (*str) {
        fb_put_char(*str);
        str++;
    }
}

/* print a signed decimal number */
void fb_print_num(int num) {
    char digits[10];
    int i = 0;
    unsigned int n;

    if (num < 0) {
        fb_put_char('-');
        n = (unsigned int) (-(num + 1)) + 1;   /* also correct for INT_MIN */
    } else {
        n = (unsigned int) num;
    }

    do {
        digits[i++] = '0' + (n % 10);
        n /= 10;
    } while (n > 0);

    while (i > 0) {
        fb_put_char(digits[--i]);
    }
}

/* print a 32-bit value as 0xXXXXXXXX */
void fb_print_hex(unsigned int num) {
    const char *hex = "0123456789ABCDEF";
    fb_print("0x");
    for (int shift = 28; shift >= 0; shift -= 4) {
        fb_put_char(hex[(num >> shift) & 0xF]);
    }
}
