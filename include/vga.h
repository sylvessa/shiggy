#ifndef VGA_H
#define VGA_H

#include "globals.h"

#define VGA_WIDTH 640
#define VGA_HEIGHT 480
#define VGA_BYTES_PER_SCANLINE (VGA_WIDTH / 8) // 80
#define VGA_FB ((volatile nat8*)0xA0000)

void vga_test_stripes(void);
void vga_put_pixel(int x, int y, nat8 color);

#endif