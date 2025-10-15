; boot sector 16 bit -> 32 bit protected mode
[ORG 0x7C00]

KERNEL_OFFSET equ 0x1000 ; kernel address might change lowkey

[BITS 16]
boot: 
    mov [BOOT_DRIVE], dl
    mov bx, MSG_REAL_MODE
    call print

load_kernel:
    mov bx, MSG_LOAD_KERNEL
    call print

    mov dh, 40
    mov dl, [BOOT_DRIVE]
    mov bx, KERNEL_OFFSET
    call disk_load
    call goto_pm

%include "print.asm"
%include "disk_load.asm"
%include "protected_mode.asm"

BOOT_DRIVE db 0
MSG_REAL_MODE: db "booting from 16 bit real mode", 10, 0
MSG_LOAD_KERNEL: db "loading kernelinto memory", 10, 0

times 510-($-$$) db 0
dw 0xAA55
