OBJECTS = source/loader.o \
          source/kernel.o \
          drivers/framebuffer.o \
          drivers/pic.o \
          drivers/keyboard.o \
          drivers/interrupts.o \
          drivers/interrupt_handlers.o \
          drivers/interrupt_asm.o \
          drivers/hardware_interrupt_enabler.o

CC = gcc
CFLAGS = -I. -m32 -nostdlib -nostdinc -fno-builtin -fno-stack-protector \
         -nostartfiles -nodefaultlibs -Wall -Wextra -Werror -c
LDFLAGS = -T ./source/link.ld -melf_i386 --no-warn-rwx-segments
AS = nasm
ASFLAGS = -f elf

all: kernel.elf

kernel.elf: $(OBJECTS)
	ld $(LDFLAGS) $(OBJECTS) -o kernel.elf

os.iso: kernel.elf
	mkdir -p iso/boot/grub
	cp kernel.elf iso/boot/kernel.elf
	cp -n /opt/os/stage2_eltorito iso/boot/grub/ 2>/dev/null || true
	genisoimage -R \
				-b boot/grub/stage2_eltorito \
				-no-emul-boot \
				-boot-load-size 4 \
				-A os \
				-input-charset utf8 \
				-quiet \
				-boot-info-table \
				-o os.iso \
				iso

# No VGA window: the terminal is connected to COM1 (input and output).
# Quit with Ctrl-A then X.
run: os.iso
	qemu-system-i386 \
		-nographic \
		-boot d \
		-cdrom os.iso \
		-m 32 \
		-no-reboot \
		-no-shutdown \
		-d cpu \
		-D logQ.txt

# VGA framebuffer drawn with curses, input from the emulated PS/2 keyboard.
# Quit from a second terminal: telnet localhost 45454, then type: quit
run-curses: os.iso
	qemu-system-i386 \
		-display curses \
		-monitor telnet::45454,server,nowait \
		-serial file:serial.log \
		-boot d \
		-cdrom os.iso \
		-m 32 \
		-no-reboot \
		-no-shutdown \
		-d cpu \
		-D logQ.txt

%.o: %.c
	$(CC) $(CFLAGS) $< -o $@

%.o: %.s
	$(AS) $(ASFLAGS) $< -o $@

%.o: %.asm
	$(AS) $(ASFLAGS) $< -o $@

clean:
	rm -rf *.o source/*.o drivers/*.o kernel.elf os.iso iso/boot/kernel.elf logQ.txt serial.log

.PHONY: all run run-curses clean
