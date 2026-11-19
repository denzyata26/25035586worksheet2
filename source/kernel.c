#include "drivers/framebuffer.h"
#include "drivers/interrupts.h"
#include "drivers/hardware_interrupt_enabler.h"
#include "drivers/keyboard.h"

int string_compare(const char *s1, const char *s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(const unsigned char *)s1 - *(const unsigned char *)s2;
}

void print_banner(void) {
    fb_set_color(FB_LIGHT_CYAN, FB_BLACK);
    fb_print("+--------------------------------------------------+\n");
    fb_print("|                  DENZYOS KERNEL                  |\n");
    fb_print("|         ID: 25035586 - Ata Deniz Onursoy         |\n");
    fb_print("|          UFCFWK-15-2 Worksheet 2 Part 2          |\n");
    fb_print("+--------------------------------------------------+\n\n");
    fb_set_color(FB_WHITE, FB_BLACK);
}

void parse_command(char *cmd) {
    if (string_compare(cmd, "help") == 0) {
        fb_set_color(FB_LIGHT_GREEN, FB_BLACK);
        fb_print("Available Commands:\n");
        fb_set_color(FB_WHITE, FB_BLACK);
        fb_print("  help      - Display this help menu\n");
        fb_print("  clear     - Clear the terminal screen\n");
        fb_print("  version   - Show kernel version and student info\n");
        fb_print("  echo      - Print user text (usage: echo [text])\n");
    } else if (string_compare(cmd, "clear") == 0) {
        fb_clear();
        print_banner();
    } else if (string_compare(cmd, "version") == 0) {
        fb_set_color(FB_LIGHT_YELLOW, FB_BLACK);
        fb_print("DenzyOS v1.0.0 [ID: 25035586 - Ata Deniz Onursoy]\n");
        fb_set_color(FB_WHITE, FB_BLACK);
    } else if (cmd[0] == 'e' && cmd[1] == 'c' && cmd[2] == 'h' && cmd[3] == 'o' && (cmd[4] == ' ' || cmd[4] == '\0')) {
        char *args = &cmd[4];
        while (*args == ' ') args++;
        fb_print(args);
        fb_print("\n");
    } else if (cmd[0] != '\0') {
        fb_set_color(FB_LIGHT_RED, FB_BLACK);
        fb_print("Error: Unknown command. Type 'help' for assistance.\n");
        fb_set_color(FB_WHITE, FB_BLACK);
    }
}

void kmain(void) {
    init_serial(); // Initialize COM1 serial port for terminal input/output

    fb_set_color(FB_WHITE, FB_BLACK);
    fb_clear();

    print_banner();

    interrupts_install_idt();
    enable_hardware_interrupts();

    char cmd_buf[128];

    while (1) {
        fb_set_color(FB_LIGHT_CYAN, FB_BLACK);
        fb_print("25035586@denzyos:~# ");
        fb_set_color(FB_WHITE, FB_BLACK);

        readline(cmd_buf, 128);
        fb_print("\n");

        parse_command(cmd_buf);
    }
}
