section .text

extern interrupt_handler

%macro isr_no_err_stub 1
isr_stub_%+%1:
    push qword 0        ; fake error code
    push qword %1       ; vector
    jmp isr_common
%endmacro

%macro isr_err_stub 1
isr_stub_%+%1:
    push qword %1       ; vector (error code already pushed by CPU)
    jmp isr_common
%endmacro

isr_common:
    push rax
    push rbx
    push rcx
    push rdx
    push rsi
    push rdi
    push rbp
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15

    mov rdi, rsp        ; first arg = pointer to frame
    call interrupt_handler

    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rbp
    pop rdi
    pop rsi
    pop rdx
    pop rcx
    pop rbx
    pop rax
    add rsp, 16         ; discard vector + error_code
    iretq

isr_no_err_stub 0
isr_no_err_stub 1
isr_no_err_stub 2
isr_no_err_stub 3
isr_no_err_stub 4
isr_no_err_stub 5
isr_no_err_stub 6
isr_no_err_stub 7
isr_err_stub    8
isr_no_err_stub 9
isr_err_stub    10
isr_err_stub    11
isr_err_stub    12
isr_err_stub    13
isr_err_stub    14
isr_no_err_stub 15
isr_no_err_stub 16
isr_err_stub    17
isr_no_err_stub 18
isr_no_err_stub 19
isr_no_err_stub 20
isr_no_err_stub 21
isr_no_err_stub 22
isr_no_err_stub 23
isr_no_err_stub 24
isr_no_err_stub 25
isr_no_err_stub 26
isr_no_err_stub 27
isr_no_err_stub 28
isr_no_err_stub 29
isr_err_stub    30
isr_no_err_stub 31
isr_no_err_stub 32   ; IRQ0  - PIT timer
isr_no_err_stub 33   ; IRQ1  - PS/2 keyboard
isr_no_err_stub 34   ; IRQ2  - cascade (never fires)
isr_no_err_stub 35   ; IRQ3  - COM2
isr_no_err_stub 36   ; IRQ4  - COM1
isr_no_err_stub 37   ; IRQ5  - LPT2
isr_no_err_stub 38   ; IRQ6  - floppy
isr_no_err_stub 39   ; IRQ7  - LPT1 / spurious
isr_no_err_stub 40   ; IRQ8  - RTC
isr_no_err_stub 41   ; IRQ9  - ACPI
isr_no_err_stub 42   ; IRQ10 - open
isr_no_err_stub 43   ; IRQ11 - open
isr_no_err_stub 44   ; IRQ12 - PS/2 mouse
isr_no_err_stub 45   ; IRQ13 - FPU
isr_no_err_stub 46   ; IRQ14 - ATA primary
isr_no_err_stub 47   ; IRQ15 - ATA secondary

; 48-254: general device vectors
%assign i 48
%rep 207
    isr_no_err_stub i
%assign i i+1
%endrep

isr_no_err_stub 255  ; LAPIC spurious

section .data
global isr_stub_table
isr_stub_table:
%assign i 0
%rep    256
    dq isr_stub_%+i
%assign i i+1
%endrep