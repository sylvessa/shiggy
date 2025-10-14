#ifndef IDT_H
#define IDT_H

#include "types.h"
#include "lib/memory.h"

void set_idt_gate(int n, nat32 handler);
void set_idt();

#define KERNEL_CS 0x08 // gdt.asm

typedef struct {
    nat16 low_offset;
    nat16 sel;
    nat8 always0;
    nat8 flags;
    nat16 high_offset;
} __attribute__((packed)) idt_gate_t;

typedef struct {
    nat16 limit;
    nat32 base;
} __attribute__((packed)) idt_register_t;

#define IDT_ENTRIES 256

#endif