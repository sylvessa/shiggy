#include "globals.h"
#include "graphics/main.h"

void gfx_draw_pixel(int x, int y, nat8 color)
{
	if (x < 0 || x >= VGA_WIDTH || y < 0 || y >= VGA_HEIGHT)
		return;
	volatile nat8 *fb = VGA_FB;
	int bytes_per_line = VGA_BYTES_PER_SCANLINE;

	for (int plane = 0; plane < 4; plane++)
	{
		int mask = 1 << plane;
		set_map_mask(mask);
		int base = x / 8;
		int bit = 7 - (x % 8);
		if ((color >> plane) & 1)
			fb[y * bytes_per_line + base] |= (1 << bit);
		else
			fb[y * bytes_per_line + base] &= ~(1 << bit);
	}
	set_map_mask(0x0F);
}

void draw_thick_pixel(int x, int y, nat8 color, int thickness)
{
	int half = thickness / 2;
	int x_start = x - half;
	int x_end = x + half;
	int y_start = y - half;
	int y_end = y + half;

	if (x_start < 0)
		x_start = 0;
	if (y_start < 0)
		y_start = 0;
	if (x_end >= VGA_WIDTH)
		x_end = VGA_WIDTH - 1;
	if (y_end >= VGA_HEIGHT)
		y_end = VGA_HEIGHT - 1;

	for (int py = y_start; py <= y_end; py++)
	{
		int byte_start = x_start / 8;
		int byte_end = x_end / 8;

		for (int plane = 0; plane < 4; plane++)
		{
			set_map_mask((color == 0) ? 0x0F : color);
			volatile nat8 *fb = VGA_FB + py * VGA_BYTES_PER_SCANLINE;

			for (int bx = byte_start; bx <= byte_end; bx++)
			{
				int mask = 0xFF;

				if (bx == byte_start)
					mask &= 0xFF >> (x_start % 8);
				if (bx == byte_end)
					mask &= 0xFF << (7 - (x_end % 8));

				if ((color == 0) ? color & (1 << plane) : color)
					fb[bx] |= mask;
				else
					fb[bx] &= ~mask;
			}
		}
	}

	set_map_mask(color);
}

void gfx_draw_line(int x0, int y0, int x1, int y1, nat8 color, int thickness)
{
	if (thickness < 1)
		thickness = 1;

	int dx = x1 - x0;
	int dy = y1 - y0;
	int sx = dx >= 0 ? 1 : -1;
	int sy = dy >= 0 ? 1 : -1;
	dx = dx >= 0 ? dx : -dx;
	dy = dy >= 0 ? dy : -dy;

	if (dx > dy)
	{
		int err = dx / 2;
		for (; x0 != x1; x0 += sx)
		{
			draw_thick_pixel(x0, y0, color, thickness);
			err -= dy;
			if (err < 0)
			{
				y0 += sy;
				err += dx;
			}
		}
	}
	else
	{
		int err = dy / 2;
		for (; y0 != y1; y0 += sy)
		{
			draw_thick_pixel(x0, y0, color, thickness);
			err -= dx;
			if (err < 0)
			{
				x0 += sx;
				err += dy;
			}
		}
	}
	draw_thick_pixel(x1, y1, color, thickness);
}
