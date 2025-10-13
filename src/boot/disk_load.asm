disk_load:
    push dx
    mov ah, 0x02 ; bios readsector
    mov al, dh
    mov ch, 0x00
    mov dh, 0x00
    mov cl, 0x02

    ;; Hey bios go read fatty
    int 0x13

    jc disk_error

    pop dx
    cmp dh, al
    jne disk_error
    ret

disk_error:
    mov bx, MSG_DISK_ERROR
    call print
    jmp $

MSG_DISK_ERROR db "disk read error", 10, 0