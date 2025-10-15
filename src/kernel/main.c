#include "globals.h"

#define SCALE 2

static inline uint16_t rgb565(uint8_t r, uint8_t g, uint8_t b) {
	return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}

const uint8_t font[128][8] = {
	[32] = {0,0,0,0,0,0,0,0},
	[36] = {0x10,0x38,0x54,0x10,0x38,0x54,0x38,0x10},
	[64] = {0x3C,0x42,0x5A,0x56,0x5E,0x40,0x3C,0x00},
	[101] = {0x00,0x00,0x3C,0x42,0x7E,0x40,0x3C,0x00},
	[111] = {0x00,0x00,0x3C,0x42,0x42,0x42,0x3C,0x00},
	[115] = {0x00,0x00,0x3E,0x40,0x3C,0x02,0x7C,0x00},
	[114] = {0x00,0x00,0x5C,0x62,0x40,0x40,0x40,0x00},
	[116] = {0x10,0x10,0x7C,0x10,0x10,0x12,0x0C,0x00},
	[126] = {0x00,0x00,0x20,0x52,0x0C,0x00,0x00,0x00},
	[58] = {0x00,0x18,0x18,0x00,0x00,0x18,0x18,0x00},
};

void draw_char(volatile uint16_t* fb, char c, int cx, int cy, uint16_t color) {
	for (int row = 0; row < 8; row++) {
		uint8_t bits = font[(int)c][row];
		for (int col = 0; col < 8; col++) {
			if (bits & (1 << (7 - col))) {
				for (int dy = 0; dy < SCALE; dy++) {
					for (int dx = 0; dx < SCALE; dx++) {
						int px = cx + col * SCALE + dx;
						int py = cy + row * SCALE + dy;
						if (px < WIDTH && py < HEIGHT)
							fb[py * WIDTH + px] = color;
					}
				}
			}
		}
	}
}

void draw_string(volatile uint16_t* fb, const char* s, int x, int y, uint16_t color) {
	while (*s) {
		draw_char(fb, *s, x, y, color);
		x += 8 * SCALE;
		s++;
	}
}

void main() {
	volatile uint16_t* fb = (uint16_t*)VESA_ADDRESS;

	uint16_t green = rgb565(0, 255, 0);
	uint16_t blue = rgb565(0, 120, 255);
	uint16_t gray = rgb565(180, 180, 180);
	uint16_t white = rgb565(255, 255, 255);

	int x = 10, y = 10;

	draw_string(fb, "root", x, y, green); x += 4 * 8 * SCALE;
	draw_string(fb, "@", x, y, white); x += 8 * SCALE;
	draw_string(fb, "os", x, y, green); x += 2 * 8 * SCALE;
	draw_string(fb, ":", x, y, white); x += 8 * SCALE;
	draw_string(fb, "~", x, y, blue); x += 8 * SCALE;
	draw_string(fb, "$", x, y, gray); x += 8 * SCALE;
	draw_string(fb, " test", x, y, white);

	while (1) {}
}
