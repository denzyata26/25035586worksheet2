#ifndef INCLUDE_KEYBOARD_H
#define INCLUDE_KEYBOARD_H

#include "types.h"

#define KEYBOARD_MAX_ASCII 83

/* longest line the user can type (readline buffers need one extra byte) */
#define KEYBOARD_LINE_MAX 127

/* PS/2 keyboard (Task 1) */
u8int keyboard_read_scan_code(void);
u8int keyboard_scan_code_to_ascii(u8int scan_code);

/* circular input buffer, filled by the interrupt handler (Task 2) */
void keyboard_buffer_put(u8int c);
u8int getc(void);
int readline(char *buf, int max_len);

/* COM1 serial port: input source when qemu runs with -nographic */
void init_serial(void);
int serial_received(void);
u8int serial_read_byte(void);

#endif /* INCLUDE_KEYBOARD_H */
