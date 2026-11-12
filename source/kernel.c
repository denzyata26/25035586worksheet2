#include "../drivers/framebuffer.h"

int sum_of_three(int arg1, int arg2, int arg3) {
    return arg1 + arg2 + arg3;
}

int multiply(int a, int b) {
    return a * b;
}

int max_val(int a, int b) {
    return (a > b) ? a : b;
}

void kmain(void) {
    fb_set_color(FB_WHITE, FB_BLACK);
    fb_clear();

    fb_move(0, 0);
    fb_set_color(FB_LIGHT_GREEN, FB_BLACK);
    fb_print("Tiny OS - Worksheet 2 Part 1 (25035586)");

    fb_move(0, 2);
    fb_set_color(FB_LIGHT_CYAN, FB_BLACK);
    fb_print("Task 1 - Bootloader check:");
    fb_move(2, 3);
    fb_set_color(FB_WHITE, FB_BLACK);
    fb_print("EAX = ");
    fb_print_hex(0xCAFEBABE);

    fb_move(0, 5);
    fb_set_color(FB_LIGHT_CYAN, FB_BLACK);
    fb_print("Task 2 - C function tests:");

    fb_move(2, 6);
    fb_set_color(FB_WHITE, FB_BLACK);
    fb_print("sum_of_three(10, 20, 30) = ");
    fb_print_num(sum_of_three(10, 20, 30));

    fb_move(2, 7);
    fb_print("multiply(6, 7)           = ");
    fb_print_num(multiply(6, 7));

    fb_move(2, 8);
    fb_print("max_val(42, 99)          = ");
    fb_print_num(max_val(42, 99));

    fb_move(0, 10);
    fb_set_color(FB_LIGHT_GREEN, FB_BLACK);
    fb_print("Task 3 - Framebuffer 2D API & cursor I/O working.");

    fb_move(0, 12);
}
