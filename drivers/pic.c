#include "pic.h"
#include "io.h"

void pic_remap(s32int offset1, s32int offset2) {
    outb(PIC_1_COMMAND, PIC_ICW1_INIT | PIC_ICW1_ICW4);
    outb(PIC_2_COMMAND, PIC_ICW1_INIT | PIC_ICW1_ICW4);

    outb(PIC_1_DATA, (u8int) offset1);
    outb(PIC_2_DATA, (u8int) offset2);

    outb(PIC_1_DATA, 0x04);
    outb(PIC_2_DATA, 0x02);

    outb(PIC_1_DATA, PIC_ICW4_8086);
    outb(PIC_2_DATA, PIC_ICW4_8086);

    outb(PIC_1_DATA, 0xFF);
    outb(PIC_2_DATA, 0xFF);
}

void pic_acknowledge(u32int interrupt) {
    if (interrupt < PIC_1_OFFSET || interrupt > PIC_2_END) {
        return;
    }
    if (interrupt >= PIC_2_OFFSET) {
        outb(PIC_2_COMMAND_PORT, PIC_ACKNOWLEDGE);
    }
    outb(PIC_1_COMMAND_PORT, PIC_ACKNOWLEDGE);
}
