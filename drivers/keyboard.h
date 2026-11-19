#ifndef INCLUDE_KEYBOARD_H
#define INCLUDE_KEYBOARD_H

#include "types.h"

#define KEYBOARD_MAX_ASCII 128

void init_serial(void);
u8int getc(void);
int readline(char *buf, int max_len);
u8int keyboard_read_scan_code(void);
void keyboard_buffer_put(u8int c);
u8int keyboard_scan_code_to_ascii(u8int scan_code);

#endif /* INCLUDE_KEYBOARD_H */
