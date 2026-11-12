global loader
global outb
global inb
extern kmain

MAGIC_NUMBER      equ 0x1BADB002
FLAGS             equ 0x0
CHECKSUM          equ -MAGIC_NUMBER
KERNEL_STACK_SIZE equ 4096

section .multiboot
align 4
    dd MAGIC_NUMBER
    dd FLAGS
    dd CHECKSUM

section .text
align 4

outb:
    mov al, [esp + 8]
    mov dx, [esp + 4]
    out dx, al
    ret

inb:
    mov dx, [esp + 4]
    in al, dx
    ret

loader:
    mov eax, 0xCAFEBABE
    mov esp, kernel_stack + KERNEL_STACK_SIZE
    call kmain
    mov eax, 0xCAFEBABE

.loop:
    jmp .loop

section .bss
align 4
kernel_stack:
    resb KERNEL_STACK_SIZE
