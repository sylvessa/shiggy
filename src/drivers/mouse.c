#include "drivers/mouse.h"
#include "cpu/isr.h"
#include "drivers/pic.h"
#include "drivers/vga.h"
#include "globals.h"

int mouse_x = 0;
int mouse_y = 0;

static nat8 mouse_cycle = 0;
static nat8 mouse_data;
byte mouse_packet[4];

static int prev_mouse_x = -1;
static int prev_mouse_y = -1;

static bool left_pressed = false;
static bool right_pressed = false;

static const int cursor_size = 16;

void draw_mouse_arrow(int x, int y, nat8 color) {
	for (int row = 0; row < cursor_size; row++) {
		for (int col = 0; col < cursor_size; col++) {
			if (cursor_mask[row] & (1 << (15 - col))) {
				int draw_x = x + col;
				int draw_y = y + row;
				if (draw_x >= 0 && draw_x < VGA_WIDTH && draw_y >= 0 && draw_y < VGA_HEIGHT)
					draw_char_pixel(draw_x, draw_y, color, color, 1);
			}
		}
	}
}

void erase_mouse_arrow(int x, int y) {
	for (int row = 0; row < cursor_size; row++) {
		for (int col = 0; col < cursor_size; col++) {
			if (cursor_mask[row] & (1 << (15 - col))) {
				int draw_x = x + col;
				int draw_y = y + row;
				if (draw_x >= 0 && draw_x < VGA_WIDTH && draw_y >= 0 && draw_y < VGA_HEIGHT)
					draw_char_pixel(draw_x, draw_y, 0x00, 0x00, 0);
			}
		}
	}
}

void mouse_callback() {
	if (!gui_mode) return;
	mouse_data = in_byte(0x60);

	if (mouse_cycle == 0 && !(mouse_data & 0x08))
		return;

	mouse_packet[mouse_cycle++] = mouse_data;

	if (mouse_cycle == 3) {
		mouse_cycle = 0;

		// movement
		int8 dx = (int8)mouse_packet[1];
		int8 dy = (int8)mouse_packet[2];

		mouse_x += dx;
		mouse_y -= dy;

		if (mouse_x < 0) mouse_x = 0;
		if (mouse_y < 0) mouse_y = 0;
		if (mouse_x >= VGA_WIDTH) mouse_x = VGA_WIDTH - 1;
		if (mouse_y >= VGA_HEIGHT) mouse_y = VGA_HEIGHT - 1;

		if (prev_mouse_x >= 0 && prev_mouse_y >= 0)
			erase_mouse_arrow(prev_mouse_x, prev_mouse_y);

		draw_mouse_arrow(mouse_x, mouse_y, 5);

		prev_mouse_x = mouse_x;
		prev_mouse_y = mouse_y;

		left_pressed = (mouse_packet[0] & 0x1);
		right_pressed = (mouse_packet[0] & 0x2);

		printf_at(0, 0, "              ");			   // clear
		printf_at(0, 1, "              ");			   // clear
		printf_at(0, 2, "                          "); // clear
		printf_at(0, 3, "                          "); // clear
		printf_at(0, 0, "Mouse X %d", mouse_x);
		printf_at(0, 1, "Mouse Y %d", mouse_y);
		if (left_pressed) printf_at(0, 2, "Left click press");
		if (right_pressed) printf_at(0, 3, "Right click press");
	}
}

bool is_left_pressed() {
	return left_pressed;
}
bool is_right_pressed() {
	return right_pressed;
}

void init_mouse() {
	out_byte(0x64, 0xA7);
	out_byte(0x64, 0xA8);

	while (in_byte(0x64) & 1)
		in_byte(0x60);

	register_interrupt_handler(IRQ_BASE + 12, mouse_callback);

	out_byte(0x64, 0x20);
	nat8 status = in_byte(0x60) | 3;
	out_byte(0x64, 0x60);
	out_byte(0x60, status);

	out_byte(0x64, 0xD4);
	out_byte(0x60, 0xF4);
}
