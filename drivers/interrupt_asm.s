extern interrupt_handler

section .text

%macro no_error_code_interrupt_handler 1
global interrupt_handler_%1
interrupt_handler_%1:
    push dword 0                    ; push 0 as error code
    push dword %1                   ; push the interrupt number
    jmp common_interrupt_handler    ; jump to the common handler
%endmacro

%macro error_code_interrupt_handler 1
global interrupt_handler_%1
interrupt_handler_%1:
    push dword %1                   ; push the interrupt number
    jmp common_interrupt_handler    ; jump to the common handler
%endmacro

common_interrupt_handler:
    ; save the registers
    push eax
    push ebx
    push ecx
    push edx
    push ebp
    push esi
    push edi

    ; call the C function
    call interrupt_handler

    ; restore the registers
    pop edi
    pop esi
    pop ebp
    pop edx
    pop ecx
    pop ebx
    pop eax

    ; remove the error code and interrupt number
    add esp, 8

    ; return to the code that got interrupted
    iret

no_error_code_interrupt_handler 33  ; IRQ1 - keyboard
no_error_code_interrupt_handler 36  ; IRQ4 - COM1 serial port
