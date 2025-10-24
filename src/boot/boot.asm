[org 0x7c00]

start:
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7c00

    mov si, msg
    call print_string

    mov	ax, 0x0012
	int	0x10

    ; load protected mode
    cli
    lgdt [gdt_descriptor] ; load gdt
    mov eax, cr0
    or eax, 1 ; set PE bit
    mov cr0, eax
    jmp 0x08:pm_entry ; far jump to flush prefetch

[BITS 32]
    pm_entry:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, 0x9fc00 ; stack

    jmp 0x7e00

print_string:
    mov ah, 0x0e
.loop:
    lodsb
    or al, al
    jz .done
    int 0x10
    jmp .loop
.done:
    ret

msg db "we a re", 0x0d, 0x0a, 0

gdt_start:
    dq 0x0000000000000000 ; null descriptor
    dq 0x00cf9a000000ffff ; code segment
    dq 0x00cf92000000ffff ; data segment
gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start

times 510-($-$$) db 0
dw 0xAA55
