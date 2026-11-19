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
LDFLAGS = -T ./source/link.ld -melf_i386 --no-warn-rwx-segments --no-warn-rwx-segments
AS = nasm
ASFLAGS = -f elf

all: kernel.elf

kernel.elf: $(OBJECTS)
	ld $(LDFLAGS) $(OBJECTS) -o kernel.elf

os.iso: kernel.elf
	cp kernel.elf iso/boot/kernel.elf
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

run: os.iso
	qemu-system-i386 \
		-boot d \
		-cdrom os.iso \
		-m 32 \
		-nographic \
		-serial mon:stdio \
		-no-reboot \
		-no-shutdown

%.o: %.c
	$(CC) $(CFLAGS) $< -o $@

%.o: %.s
	$(AS) $(ASFLAGS) $< -o $@

%.o: %.asm
	$(AS) $(ASFLAGS) $< -o $@

clean:
	rm -rf *.o source/*.o drivers/*.o kernel.elf os.iso iso/boot/kernel.elf
