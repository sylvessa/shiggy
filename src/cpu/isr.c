#include "globals.h"
#include "cpu/idt.h"

isr_handler_t interrupt_handlers[256];

typedef void (*isr_func_t)();

void isr_install() {
	isr_func_t isrs[ISR_COUNT] = {
		isr0,  isr1,  isr2,  isr3,  isr4,  isr5,  isr6,  isr7,
		isr8,  isr9,  isr10, isr11, isr12, isr13, isr14, isr15,
		isr16, isr17, isr18, isr19, isr20, isr21, isr22, isr23,
		isr24, isr25, isr26, isr27, isr28, isr29, isr30, isr31
	};

	for (int i = 0; i < ISR_COUNT; i++)
		set_idt_gate(i, (nat32)isrs[i]);

	pci_init();

	isr_func_t irqs[IRQ_COUNT] = {
		irq0,  irq1,  irq2,  irq3,  irq4,  irq5,  irq6,  irq7,
		irq8,  irq9,  irq10, irq11, irq12, irq13, irq14, irq15
	};

	for (int i = 0; i < IRQ_COUNT; i++)
		set_idt_gate(IRQ_BASE + i, (nat32)irqs[i]);

	set_idt();
	__asm__ __volatile__("sti");
}

static char *exception_messages[] = {
        "Division By Zero",
        "Debug",
        "Non Maskable Interrupt",
        "Breakpoint",
        "Into Detected Overflow",
        "Out of Bounds",
        "Invalid Opcode",
        "No Coprocessor",
        "Double Fault",
        "Coprocessor Segment Overrun",
        "Bad TSS",
        "Segment Not Present",
        "Stack Fault",
        "General Protection Fault",
        "Page Fault",
        "Unknown Interrupt",
        "Coprocessor Fault",
        "Alignment Check",
        "Machine Check",
        "Reserved",
        "Reserved",
        "Reserved",
        "Reserved",
        "Reserved",
        "Reserved",
        "Reserved",
        "Reserved",
        "Reserved",
        "Reserved",
        "Reserved",
        "Reserved",
        "Reserved"};

void isr_handler(registers_t r) {
    print("received interrupt: ");
    char s[3];
    dec2str(r.int_no, s);
    print(s);
    print("\n");
    print(exception_messages[r.int_no]);
    print("\n");
}

void register_interrupt_handler(nat8 n, isr_handler_t handler) {
    interrupt_handlers[n] = handler;
}

void irq_handler(registers_t r) {
    // after every interrupt we need to send an EOI to the PICs or they will not send another interrupt again
    if (r.int_no >= 40)
        out_byte(0xA0, 0x20); // save
    
    out_byte(0x20, 0x20); //master

    if (interrupt_handlers[r.int_no] != 0)
    {
        isr_handler_t handler = interrupt_handlers[r.int_no];
        handler(r);
    }
}