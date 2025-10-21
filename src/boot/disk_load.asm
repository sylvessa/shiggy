disk_load:
    push dx
    push ax
    push cx

    mov ah, 0x02 ; BIOS read sector
    mov al, dh ; number of sectors
    mov ch, 0x00 ; cylinder
    mov dh, 0x00 ; head
    mov cl, 0x02 ; starting sector

    int 0x13

    jc disk_error

    pop cx
    pop ax
    pop dx
    ret

disk_error:
    mov bx, MSG_DISK_ERROR
    call print
    jmp $

MSG_DISK_ERROR db "disk read error", 10, 0