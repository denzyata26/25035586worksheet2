#include "drivers/framebuffer.h"
#include "drivers/interrupts.h"
#include "drivers/hardware_interrupt_enabler.h"
#include "drivers/keyboard.h"

/* ======================================================================
 * Worksheet 2 Part 1 - Task 2: C functions called from the kernel
 * ====================================================================== */
int sum_of_three(int arg1, int arg2, int arg3) {
    return arg1 + arg2 + arg3;
}

int multiply(int a, int b) {
    return a * b;
}

int max_val(int a, int b) {
    return (a > b) ? a : b;
}

/* ======================================================================
 * Small string helpers (there is no C library in the kernel)
 * ====================================================================== */
int string_compare(const char *s1, const char *s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(const unsigned char *)s1 - *(const unsigned char *)s2;
}

/* Parse a (possibly negative) decimal number at *str and advance *str
   past it. Returns 1 on success, 0 if there was no number. */
static int parse_int(char **str, int *out) {
    char *s = *str;
    int sign = 1;
    int value = 0;

    while (*s == ' ') s++;
    if (*s == '-') {
        sign = -1;
        s++;
    }
    if (*s < '0' || *s > '9') {
        return 0;
    }
    while (*s >= '0' && *s <= '9') {
        value = value * 10 + (*s - '0');
        s++;
    }
    *out = sign * value;
    *str = s;
    return 1;
}

static void print_error(const char *msg) {
    fb_set_color(FB_LIGHT_RED, FB_BLACK);
    fb_print(msg);
    fb_set_color(FB_WHITE, FB_BLACK);
}

/* ======================================================================
 * Worksheet 2 Part 1 - Task 2 & 3 test output
 * Exercises the C functions and the 2D framebuffer API
 * (fb_move, fb_set_color, fb_print, fb_print_num, fb_print_hex).
 * ====================================================================== */
static void print_part1_tests(void) {
    fb_set_color(FB_LIGHT_GREEN, FB_BLACK);
    fb_print("Part 1 tests\n");

    fb_set_color(FB_WHITE, FB_BLACK);
    fb_print("  sum_of_three(10, 20, 30) = ");
    fb_print_num(sum_of_three(10, 20, 30));
    fb_print("\n  multiply(6, -7)          = ");
    fb_print_num(multiply(6, -7));
    fb_print("\n  max_val(42, 99)          = ");
    fb_print_num(max_val(42, 99));
    fb_print("\n  fb_print_hex(0xCAFEBABE) = ");
    fb_print_hex(0xCAFEBABE);
    fb_print("\n  colours: ");

    fb_set_color(FB_LIGHT_RED, FB_BLACK);     fb_print("red ");
    fb_set_color(FB_LIGHT_GREEN, FB_BLACK);   fb_print("green ");
    fb_set_color(FB_LIGHT_BLUE, FB_BLACK);    fb_print("blue ");
    fb_set_color(FB_YELLOW, FB_BLUE);         fb_print("yellow-on-blue");
    fb_set_color(FB_WHITE, FB_BLACK);
    fb_print("\n");
}

/* fb_move demo: write text at fixed (x, y) positions */
static void print_fb_move_demo(void) {
    fb_clear();
    fb_set_color(FB_LIGHT_CYAN, FB_BLACK);
    fb_move(0, 0);
    fb_print("fb_move(0, 0)  top-left");
    fb_move(55, 0);
    fb_print("fb_move(55, 0) top-right");
    fb_move(30, 12);
    fb_print("fb_move(30, 12) centre");
    fb_move(0, 24);
    fb_print("fb_move(0, 24) bottom-left");
    fb_set_color(FB_WHITE, FB_BLACK);
    fb_move(0, 2);
}

/* ======================================================================
 * Worksheet 2 Part 2 - Task 3: terminal
 * ====================================================================== */
static void print_banner(void) {
    fb_set_color(FB_LIGHT_CYAN, FB_BLACK);
    fb_print("+--------------------------------------------------+\n");
    fb_print("|                  DENZYOS KERNEL                  |\n");
    fb_print("|         ID: 25035586 - Ata Deniz Onursoy         |\n");
    fb_print("|          UFCFWK-15-2 Worksheet 2 Part 2          |\n");
    fb_print("+--------------------------------------------------+\n\n");
    fb_set_color(FB_WHITE, FB_BLACK);
}

static void cmd_help(char *args);
static void cmd_echo(char *args);
static void cmd_clear(char *args);
static void cmd_version(char *args);
static void cmd_sum(char *args);
static void cmd_test(char *args);
static void cmd_move(char *args);

struct command {
    const char *name;
    const char *usage;
    void (*function)(char *args);
};

static const struct command commands[] = {
    { "help",    "help      - Display this help menu",                 cmd_help    },
    { "clear",   "clear     - Clear the terminal screen",              cmd_clear   },
    { "version", "version   - Show kernel version and student info",   cmd_version },
    { "echo",    "echo      - Print user text (usage: echo [text])",   cmd_echo    },
    { "sum",     "sum       - Call sum_of_three (usage: sum a b c)",   cmd_sum     },
    { "test",    "test      - Run the Part 1 tests again",             cmd_test    },
    { "move",    "move      - fb_move demo (clears the screen)",       cmd_move    },
};

#define COMMAND_COUNT (sizeof(commands) / sizeof(commands[0]))

static void cmd_help(__attribute__((unused)) char *args) {
    fb_set_color(FB_LIGHT_GREEN, FB_BLACK);
    fb_print("Available Commands:\n");
    fb_set_color(FB_WHITE, FB_BLACK);
    for (unsigned int i = 0; i < COMMAND_COUNT; i++) {
        fb_print("  ");
        fb_print(commands[i].usage);
        fb_print("\n");
    }
}

static void cmd_echo(char *args) {
    fb_print(args);
    fb_print("\n");
}

static void cmd_clear(__attribute__((unused)) char *args) {
    fb_clear();
    print_banner();
}

static void cmd_version(__attribute__((unused)) char *args) {
    fb_set_color(FB_LIGHT_YELLOW, FB_BLACK);
    fb_print("DenzyOS v1.0.0 [ID: 25035586 - Ata Deniz Onursoy]\n");
    fb_set_color(FB_WHITE, FB_BLACK);
}

static void cmd_sum(char *args) {
    int a, b, c;
    char *p = args;

    if (!parse_int(&p, &a) || !parse_int(&p, &b) || !parse_int(&p, &c)) {
        print_error("usage: sum a b c   (three whole numbers)\n");
        return;
    }
    fb_print("sum_of_three(");
    fb_print_num(a);
    fb_print(", ");
    fb_print_num(b);
    fb_print(", ");
    fb_print_num(c);
    fb_print(") = ");
    fb_print_num(sum_of_three(a, b, c));
    fb_print("\n");
}

static void cmd_test(__attribute__((unused)) char *args) {
    print_part1_tests();
}

static void cmd_move(__attribute__((unused)) char *args) {
    print_fb_move_demo();
}

/* Split the line into command (first word) and arguments (the rest),
   look the command up in the table and run it. */
static void execute_command(char *line) {
    char *name = line;
    char *args;

    while (*name == ' ') name++;      /* skip leading spaces */
    if (*name == '\0') {
        return;                       /* empty line: nothing to do */
    }

    args = name;
    while (*args != ' ' && *args != '\0') args++;
    if (*args == ' ') {
        *args = '\0';                 /* terminate the command name */
        args++;
        while (*args == ' ') args++;  /* skip spaces before the arguments */
    }

    for (unsigned int i = 0; i < COMMAND_COUNT; i++) {
        if (string_compare(name, commands[i].name) == 0) {
            commands[i].function(args);
            return;
        }
    }

    print_error("Error: Unknown command. Type 'help' for assistance.\n");
}

void kmain(void) {
    char line[KEYBOARD_LINE_MAX + 1];

    init_serial();

    fb_set_color(FB_WHITE, FB_BLACK);
    fb_clear();
    print_banner();
    print_part1_tests();
    fb_print("\n");

    interrupts_install_idt();
    enable_hardware_interrupts();

    while (1) {
        fb_set_color(FB_LIGHT_CYAN, FB_BLACK);
        fb_print("25035586@denzyos:~# ");
        fb_set_color(FB_WHITE, FB_BLACK);

        readline(line, sizeof(line));
        execute_command(line);
    }
}
