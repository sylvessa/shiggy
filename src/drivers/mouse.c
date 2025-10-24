#include "globals.h"
#include "drivers/mouse.h"
#include "drivers/vga.h"
#include "cpu/isr.h"
#include "drivers/pic.h"

int mouse_x = 0;
int mouse_y = 0;

static nat8 mouse_cycle = 0;
static nat8 mouse_data;
byte mouse_packet[4];


void mouse_callback() {
	mouse_data = in_byte(0x60);

	if(mouse_cycle == 0 && !(mouse_data & 0x08))
		return;
	
	mouse_packet[mouse_cycle++] = mouse_data;
	if (mouse_cycle == 3 && gui_mode) {
		mouse_cycle = 0;
		int8 dx = (int8)mouse_packet[1];
		int8 dy = (int8)mouse_packet[2];

		mouse_x += dx;
		mouse_y -= dy;

		if (mouse_x < 0) mouse_x = 0;
		if (mouse_y < 0) mouse_y = 0;
		if (mouse_x >= VGA_WIDTH) mouse_x = VGA_WIDTH - 1;
		if (mouse_y >= VGA_HEIGHT) mouse_y = VGA_HEIGHT - 1;

		draw_char_pixel(mouse_x, mouse_y, 0x0F, 0x0F, 1);
	}
}

void init_mouse() {
	out_byte(0x64, 0xA7);
	out_byte(0x64, 0xA8);

	while(in_byte(0x64) & 1) in_byte(0x60);

	register_interrupt_handler(IRQ_BASE + 12, mouse_callback);

	out_byte(0x64, 0x20);
	nat8 status = in_byte(0x60) | 2;
	out_byte(0x64, 0x60);
	out_byte(0x60, status);

	out_byte(0x64, 0xD4);
	out_byte(0x60, 0xF4);
}