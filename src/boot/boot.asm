; boot sector 16 bit -> 32 bit protected mode
[ORG 0x7C00]

KERNEL_PHYS equ 0x20000    ; physical load address of kernel
KERNEL_SEG  equ KERNEL_PHYS / 16 ; segment for real mode load

[BITS 16]
boot:
    mov [BOOT_DRIVE], dl
    mov bx, MSG_REAL_MODE
    call print

load_kernel:
    mov bx, MSG_LOAD_KERNEL
    call print

    mov	ax, 0x0012
	int 0x10

    mov dh, 40
    mov dl, [BOOT_DRIVE]

    mov ax, KERNEL_SEG
    mov es, ax
    xor bx, bx
    call disk_load

    call goto_pm

%include "print.asm"
%include "disk_load.asm"
%include "protected_mode.asm"

BOOT_DRIVE db 0
MSG_REAL_MODE: db "booting from 16 bit real mode", 10, 0
MSG_LOAD_KERNEL: db "loading kernel into memory", 10, 0

times 510-($-$$) db 0
dw 0xAA55
