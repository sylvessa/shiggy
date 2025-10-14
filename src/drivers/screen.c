#include "globals.h"

void screen_clear() {
    uint16_t *vga = (uint16_t*)VGA_ADDRESS;
	uint16_t blank = 0x0F20;

	for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++)
		vga[i] = blank;
}