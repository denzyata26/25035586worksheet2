#include "framebuffer.h"
#include "io.h"

#define FB_ADDRESS           0x000B8000
#define FB_COMMAND_PORT      0x3D4
#define FB_DATA_PORT         0x3D5
#define FB_HIGH_BYTE_COMMAND 14
#define FB_LOW_BYTE_COMMAND  15

static unsigned short cursor_x = 0;
static unsigned short cursor_y = 0;
static unsigned char fg_color = FB_WHITE;
static unsigned char bg_color = FB_BLACK;

void fb_write_cell(unsigned int i, char c, unsigned char fg, unsigned char bg) {
    char *fb = (char *) FB_ADDRESS;
    fb[i * 2] = c;
    fb[i * 2 + 1] = ((bg & 0x0F) << 4) | (fg & 0x0F);
}

void fb_move_cursor(unsigned short pos) {
    outb(FB_COMMAND_PORT, FB_HIGH_BYTE_COMMAND);
    outb(FB_DATA_PORT, ((pos >> 8) & 0x00FF));
    outb(FB_COMMAND_PORT, FB_LOW_BYTE_COMMAND);
    outb(FB_DATA_PORT, pos & 0x00FF);
}

void fb_move(unsigned short x, unsigned short y) {
    if (x >= FB_WIDTH) x = FB_WIDTH - 1;
    if (y >= FB_HEIGHT) y = FB_HEIGHT - 1;
    cursor_x = x;
    cursor_y = y;
    fb_move_cursor(y * FB_WIDTH + x);
}

void fb_set_color(unsigned char fg, unsigned char bg) {
    fg_color = fg;
    bg_color = bg;
}

void fb_clear(void) {
    for (unsigned short y = 0; y < FB_HEIGHT; y++) {
        for (unsigned short x = 0; x < FB_WIDTH; x++) {
            fb_write_cell(y * FB_WIDTH + x, ' ', fg_color, bg_color);
        }
    }
    fb_move(0, 0);
}

void fb_put_char(char c) {
    if (c == '\n') {
        cursor_x = 0;
        cursor_y++;
    } else {
        fb_write_cell(cursor_y * FB_WIDTH + cursor_x, c, fg_color, bg_color);
        cursor_x++;
        if (cursor_x >= FB_WIDTH) {
            cursor_x = 0;
            cursor_y++;
        }
    }
    if (cursor_y >= FB_HEIGHT) cursor_y = 0;
    fb_move(cursor_x, cursor_y);
}

void fb_print(const char *str) {
    for (int i = 0; str[i] != '\0'; i++) fb_put_char(str[i]);
}

void fb_print_num(int num) {
    char buf[16];
    int i = 0;
    if (num == 0) { fb_put_char('0'); return; }
    if (num < 0) { fb_put_char('-'); num = -num; }
    while (num > 0) { buf[i++] = (num % 10) + '0'; num /= 10; }
    while (i > 0) fb_put_char(buf[--i]);
}

void fb_print_hex(unsigned int num) {
    const char *hex = "0123456789ABCDEF";
    fb_print("0x");
    for (int i = 28; i >= 0; i -= 4) fb_put_char(hex[(num >> i) & 0xF]);
}
