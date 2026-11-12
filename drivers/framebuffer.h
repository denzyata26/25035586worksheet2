#ifndef INCLUDE_FRAMEBUFFER_H
#define INCLUDE_FRAMEBUFFER_H
#define FB_BLACK 0
#define FB_GREEN 2
#define FB_CYAN 3
#define FB_RED 4
#define FB_LIGHT_GREEN 10
#define FB_LIGHT_CYAN 11
#define FB_WHITE 15
#define FB_WIDTH 80
#define FB_HEIGHT 25
void fb_write_cell(unsigned int i, char c, unsigned char fg, unsigned char bg);
void fb_move_cursor(unsigned short pos);
void fb_move(unsigned short x, unsigned short y);
void fb_set_color(unsigned char fg, unsigned char bg);
void fb_clear(void);
void fb_put_char(char c);
void fb_print(const char *str);
void fb_print_num(int num);
void fb_print_hex(unsigned int num);
#endif
