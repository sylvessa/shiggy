#ifndef ISR_H
#define ISR_H

#include "types.h"
#include "drivers/pic.h"

#define ISR_COUNT 32

#define IRQ_BASE 32
#define IRQ_COUNT 16

typedef struct
{
	nat32 ds;
	nat32 edi, esi, ebp, esp, ebx, edx, ecx, eax;
	nat32 int_no; // interput
	nat32 err_code; // error code 
	nat32 eip, cs, eflags, useresp, ss; // cpu execution state
} registers_t;

typedef void (*isr_handler_t)(registers_t);

void isr_install();
void isr_handler(registers_t r);
void register_interrupt_handler(nat8 n, isr_handler_t handler);

// macoro
#define DECLARE_ISR(n) extern void isr##n();
#define DECLARE_IRQ(n) extern void irq##n();

// cpu exceptions
// these are reserved for CPU faults like page faults etc etc etc
DECLARE_ISR(0)  DECLARE_ISR(1)  DECLARE_ISR(2)  DECLARE_ISR(3)
DECLARE_ISR(4)  DECLARE_ISR(5)  DECLARE_ISR(6)  DECLARE_ISR(7)
DECLARE_ISR(8)  DECLARE_ISR(9)  DECLARE_ISR(10) DECLARE_ISR(11)
DECLARE_ISR(12) DECLARE_ISR(13) DECLARE_ISR(14) DECLARE_ISR(15)
DECLARE_ISR(16) DECLARE_ISR(17) DECLARE_ISR(18) DECLARE_ISR(19)
DECLARE_ISR(20) DECLARE_ISR(21) DECLARE_ISR(22) DECLARE_ISR(23)
DECLARE_ISR(24) DECLARE_ISR(25) DECLARE_ISR(26) DECLARE_ISR(27)
DECLARE_ISR(28) DECLARE_ISR(29) DECLARE_ISR(30) DECLARE_ISR(31)

// hardware interrupts
DECLARE_IRQ(0)  // system timer
DECLARE_IRQ(1)  // keyboard
DECLARE_IRQ(2)  // cascaded from IRQs 8-15
DECLARE_IRQ(6)  // floppy disk
DECLARE_IRQ(8)  // real-time clock
DECLARE_IRQ(13) // CPU co-processor
DECLARE_IRQ(14) // primary ATA channel (hdd or cd)
DECLARE_IRQ(15) // secondary ATA channel

#endif
