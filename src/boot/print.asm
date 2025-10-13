print:
    mov ah, 0x0E

print_while:
    mov al, [bx]
    or al, al

    jz print_done
    int 0x10
    add bx, 1
    jmp print_while

print_done:
    ret