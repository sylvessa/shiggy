#include "globals.h"

byte in_byte(nat16 port) {
    nat8 result;
    __asm__("in %%dx, %%al" : "=a"(result) : "d"(port));
    return result;
}

void out_byte(nat16 port, byte data) {
    __asm__ __volatile__("out %%al, %%dx" :: "a"(data), "d"(port));
}

b16 in_b16(nat16 port) {
    b16 result;
    __asm__("in %%dx, %%ax" : "=a"(result) : "d"(port));
    return result;
}

void out_b16(nat16 port, b16 data) {
    __asm__ __volatile__("out %%al, %%dx" :: "a"(data), "d"(port));
}

void io_wait() {
    __asm__ __volatile__("outb %%al, $0x80" :: "a"(0));
}