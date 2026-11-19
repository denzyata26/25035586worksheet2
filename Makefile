ASM = nasm
CC = gcc
CFLAGS = -m32 -nostdlib -nostdinc -fno-builtin -fno-stack-protector -nostartfiles -nodefaultlibs -Wall -Wextra -Werror -c
LDFLAGS = -T source/link.ld -melf_i386 -n -z noexecstack --no-warn-rwx-segments
ASFLAGS = -f elf32

all: kernel.elf os.iso

loader.o: source/loader.asm
	$(ASM) $(ASFLAGS) source/loader.asm -o loader.o

kernel.o: source/kernel.c
	$(CC) $(CFLAGS) source/kernel.c -o kernel.o

framebuffer.o: drivers/framebuffer.c
	$(CC) $(CFLAGS) drivers/framebuffer.c -o framebuffer.o

kernel.elf: loader.o kernel.o framebuffer.o
	ld $(LDFLAGS) loader.o kernel.o framebuffer.o -o kernel.elf
	mkdir -p iso/boot/grub
	cp kernel.elf iso/boot/
	cp /opt/os/stage2_eltorito iso/boot/grub/

os.iso: kernel.elf
	genisoimage -R -b boot/grub/stage2_eltorito -no-emul-boot -boot-load-size 4 -A os -input-charset utf8 -quiet -boot-info-table -o os.iso iso

run: os.iso
	@pkill -u $$(whoami) -f "[q]emu-system-i386" || true
	export TERM=xterm-256color; qemu-system-i386 -display curses -monitor telnet::45454,server,nowait -serial mon:stdio -boot d -cdrom os.iso -m 32 -d cpu -D logQ.txt

clean:
	rm -rf *.o kernel.elf iso/boot/kernel.elf iso/boot/grub/stage2_eltorito os.iso logQ.txt
