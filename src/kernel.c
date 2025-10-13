#include <stdint.h>

#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define VGA_ADDRESS 0xB8000

void kmain()
{
	uint16_t *vga = (uint16_t*)VGA_ADDRESS;
	uint16_t blank = 0x0F20; // 0x0F = white on black, 0x20 = space

	for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++)
		vga[i] = blank;

	while (1) {}
}
