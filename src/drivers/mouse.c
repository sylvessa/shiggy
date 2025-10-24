#include "globals.h"
#include "drivers/mouse.h"
#include "drivers/vga.h"
#include "cpu/isr.h"
#include "drivers/pic.h"

void mouse_callback() {
	print("mouse");
}

void init_mouse() {
	register_interrupt_handler(42, mouse_callback); // irq1
}