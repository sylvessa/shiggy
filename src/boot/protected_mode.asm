[BITS 32]

pmode:
	mov ax, DATASEG
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax

	mov ebp, 0x90000
	mov esp, ebp
	mov ss, ax

	jmp KERNEL
