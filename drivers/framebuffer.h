#ifndef INCLUDE_FRAMEBUFFER_H
#define INCLUDE_FRAMEBUFFER_H

#define FB_WIDTH  80
#define FB_HEIGHT 25

#define FB_BLACK         0
#define FB_BLUE          1
#define FB_GREEN         2
#define FB_CYAN          3
#define FB_RED           4
#define FB_MAGENTA       5
#define FB_BROWN         6
#define FB_LIGHT_GREY    7
#define FB_DARK_GREY     8
#define FB_LIGHT_BLUE    9
#define FB_LIGHT_GREEN   10
#define FB_LIGHT_CYAN    11
#define FB_LIGHT_RED     12
#define FB_LIGHT_MAGENTA 13
#define FB_YELLOW        14
#define FB_LIGHT_YELLOW  14  /* VGA colour 14 is shown as yellow */
#define FB_WHITE         15

/* low level */
void fb_write_cell(unsigned int cell, char c, unsigned char fg, unsigned char bg);
void fb_move_cursor(unsigned short pos);

/* 2D API */
void fb_move(unsigned short x, unsigned short y);
void fb_set_color(unsigned char fg, unsigned char bg);
void fb_clear(void);
void fb_put_char(char c);
void fb_newline(void);
void fb_backspace(void);
void fb_print(const char *str);
void fb_print_num(int num);
void fb_print_hex(unsigned int num);

#endif /* INCLUDE_FRAMEBUFFER_H */
