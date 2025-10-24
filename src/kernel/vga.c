#include "globals.h"
#include "vga.h"

static inline void outb(nat16 port, nat8 val) {
	__asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

static inline void seq_write(nat8 index, nat8 value) {
	outb(0x3C4, index);
	outb(0x3C5, value);
}

static void set_map_mask(nat8 mask) {
	seq_write(0x02, mask);
}

void vga_test_stripes(void) {
    volatile nat8 *fb = VGA_FB;
    const int bytes_per_line = VGA_BYTES_PER_SCANLINE;

    for (int x_byte = 0; x_byte < VGA_BYTES_PER_SCANLINE; x_byte++) {
        int stripe_index = (x_byte / 10) & 0xF; // change divisor to widen/narrow stripes
        nat8 color = (nat8)stripe_index;

        for (int plane = 0; plane < 4; plane++) {
            nat8 mask = (1 << plane);
            set_map_mask(mask);
            nat8 fill = ( (color >> plane) & 1 ) ? 0xFF : 0x00;

            int base = x_byte;
            for (int y = 0; y < VGA_HEIGHT; y++) {
                fb[y * bytes_per_line + base] = fill;
            }
        }
    }

    set_map_mask(0x0F);
}

void vga_put_pixel(int x, int y, nat8 color) {
	volatile nat8 *fb = VGA_FB;
	int byte_offset = x / 8;
	int bit = 7 - (x % 8); // pixel within byte

	for (int plane = 0; plane < 4; plane++) {
		set_map_mask(1 << plane);
		nat8 old = fb[y * VGA_BYTES_PER_SCANLINE + byte_offset];
		if ((color >> plane) & 1) {
			old |= (1 << bit);
		} else {
			old &= ~(1 << bit);
		}
		fb[y * VGA_BYTES_PER_SCANLINE + byte_offset] = old;
	}

	set_map_mask(0x0F);
}
