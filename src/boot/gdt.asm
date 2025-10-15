[BITS 16]

setup_gdt:
    cli
    lgdt [GDTR]
    mov eax, cr0
    or eax, 0x01
    mov cr0, eax
        
    ret

GDT:
    GDT.Null:
        dd 0x00
        dd 0x00

        GDT.Code:
            dw  0xFFFF
            dw  0x00
            db  0x00
            db  0x9A
            db  0xCF
            db  0x00

        GDT.Data:
            dw  0xFFFF
            dw  0x00
            db  0x00
            db  0x92
            db  0xCF
            db  0x00

GDTR:
    dw GDTR - GDT - 1
    dd GDT

CODESEG equ GDT.Code - GDT
DATASEG equ GDT.Data - GDT
