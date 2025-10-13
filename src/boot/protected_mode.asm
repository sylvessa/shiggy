goto_pm:
    cli
    lgdt [gdt_descriptor]

    mov eax, cr0
    or eax, 1
    mov cr0, eax

    jmp CODE_SEG:init_pm

%include "gdt.asm"

[BITS 32]
init_pm:
    ; WE ARE IN PM NOW!! YAYAYAYAYAYA
    mov ax, DATA_SEG
    mov ds, ax
    mov ss, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    ;; set stack
    mov ebp, 0x90000
    mov esp, ebp

    ;; goto kernel!!!
    call KERNEL_OFFSET