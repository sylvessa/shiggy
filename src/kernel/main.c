#include "globals.h"
#include <stdint.h>

volatile uint8_t* vga = (uint8_t*)VGA_ADDRESS;

void set_pixel(int x, int y, uint8_t color) {
	if (x >= 0 && x < WIDTH && y >= 0 && y < HEIGHT)
		vga[y * WIDTH + x] = color;
}

void draw_block(int x, int y, int size, uint8_t color) {
	for (int dy = 0; dy < size; dy++) {
		for (int dx = 0; dx < size; dx++) {
			set_pixel(x + dx, y + dy, color);
		}
	}
}

void main() {
	uint8_t color = 0;
	for (int y = 0; y < HEIGHT; y += BLOCK) {
		for (int x = 0; x < WIDTH; x += BLOCK) {
			draw_block(x, y, BLOCK, color);
			color++;
			if (color > 255) color = 0;
		}
	}

	while (1) {}
}