#include "globals.h"
#include "graphics/main.h"

void gfx_draw_pixel(int x, int y, nat8 color) {
	if(x < 0 || x >= VGA_WIDTH || y < 0 || y >= VGA_HEIGHT) return;
	volatile nat8 *fb = VGA_FB;
	int bytes_per_line = VGA_BYTES_PER_SCANLINE;

	for(int plane=0; plane<4; plane++) {
		int mask = 1 << plane;
		set_map_mask(mask);
		int base = x / 8;
		int bit = 7 - (x % 8);
		if((color >> plane) & 1)
			fb[y * bytes_per_line + base] |= (1 << bit);
		else
			fb[y * bytes_per_line + base] &= ~(1 << bit);
	}
	set_map_mask(0x0F);
}

void draw_thick_pixel(int x, int y, nat8 color, int thickness) {
		for(int tx = -thickness/2; tx <= thickness/2; tx++)
			for(int ty = -thickness/2; ty <= thickness/2; ty++)
				gfx_draw_pixel(x+tx, y+ty, color);
	}

void gfx_draw_line(int x0, int y0, int x1, int y1, nat8 color, int thickness) {
	if(thickness < 1) thickness = 1;

	int dx = x1 - x0;
	int dy = y1 - y0;
	int sx = dx >= 0 ? 1 : -1;
	int sy = dy >= 0 ? 1 : -1;
	dx = dx >= 0 ? dx : -dx;
	dy = dy >= 0 ? dy : -dy;

	if(dx > dy) {
		int err = dx/2;
		for(; x0 != x1; x0 += sx) {
			draw_thick_pixel(x0, y0, color, thickness);
			err -= dy;
			if(err < 0) { y0 += sy; err += dx; }
		}
	} else {
		int err = dy/2;
		for(; y0 != y1; y0 += sy) {
			draw_thick_pixel(x0, y0, color, thickness);
			err -= dx;
			if(err < 0) { x0 += sx; err += dy; }
		}
	}
	draw_thick_pixel(x1, y1, color, thickness);
}
