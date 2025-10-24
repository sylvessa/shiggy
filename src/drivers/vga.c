#include "globals.h"
#include "font/main.h"

static int cursor_x = 0;
static int cursor_y = 0;
static nat8 fg_color = 0x0F;
static nat8 bg_color = 0x00;
static int cursor_blink = 0;
static nat32 cursor_tick = 0;

static inline void seq_write(nat8 index, nat8 value) {
	out_byte(0x3C4, index);
	out_byte(0x3C5, value);
}

static void set_map_mask(nat8 mask) {
	seq_write(0x02, mask);
}

void draw_char_pixel(int px, int py, nat8 fg, nat8 bg, int set) {
	volatile nat8 *fb = VGA_FB;
	int bytes_per_line = VGA_BYTES_PER_SCANLINE;
	nat8 color = set ? fg : bg;

	for(int plane = 0; plane < 4; plane++) {
		set_map_mask(1 << plane);
		int base = px / 8;
		int bit = 7 - (px % 8);
		if((color >> plane) & 1)
			fb[py * bytes_per_line + base] |= (1 << bit);
		else
			fb[py * bytes_per_line + base] &= ~(1 << bit);
	}
	set_map_mask(0x0F);
}

void vga_draw_char(int x, int y, char c, nat8 fg, nat8 bg) {
	volatile nat8 *fb = VGA_FB;
	int bytes_per_line = VGA_BYTES_PER_SCANLINE;

	for(int row=0; row<HFONT; row++) {
		byte bits = isoFont[(nat8)c * HFONT + row];
		int base = x / 8;

		for(int plane=0; plane<4; plane++) {
			set_map_mask(1 << plane);
			nat8 byte_val = 0;
			for(int bit=0; bit<8; bit++) {
				int mask = 1 << (7-bit);
				int pixel_on = (bits & mask) != 0;
				if(pixel_on)
					byte_val |= ((fg >> plane) & 1) << (7-bit);
				else
					byte_val |= ((bg >> plane) & 1) << (7-bit);
			}
			fb[(y + row) * bytes_per_line + base] = byte_val;
		}
	}
	set_map_mask(0x0F);
}


static void clear_char_area(int cx, int cy, nat8 color) {
	for(int y = 0; y < HFONT; y++) {
		for(int x = 0; x < WFONT; x++) {
			draw_char_pixel(cx + x, cy + y, color, color, 0);
		}
	}
}

void screen_clear() {
	for(int y = 0; y < VGA_HEIGHT / HFONT; y++) {
		for(int x = 0; x < VGA_WIDTH / WFONT; x++) {
			vga_draw_char(x * WFONT, y * HFONT, ' ', fg_color, bg_color);
		}
	}
	cursor_x = 0;
	cursor_y = 0;
}

void vga_scroll() {
	int max_rows = VGA_HEIGHT / HFONT;
	if(cursor_y >= max_rows) {
		int lines_to_move = cursor_y - max_rows + 1;
		int pixels_to_move = lines_to_move * HFONT;
		memmove((void*)VGA_FB, (const void*)(VGA_FB + pixels_to_move * VGA_BYTES_PER_SCANLINE),
			VGA_HEIGHT * VGA_BYTES_PER_SCANLINE - pixels_to_move * VGA_BYTES_PER_SCANLINE);

		for(int y = max_rows - lines_to_move; y < max_rows; y++) {
			for(int x = 0; x < VGA_WIDTH / WFONT; x++) {
				vga_draw_char(x * WFONT, y * HFONT, ' ', fg_color, bg_color);
			}
		}
		cursor_y = max_rows - 1;
	}
}

void print_char(char c, nat8 fg, nat8 bg) {
	clear_char_area(cursor_x * WFONT, cursor_y * HFONT, bg);

	if(c == '\n') {
		cursor_x = 0;
		cursor_y++;
	} else {
		vga_draw_char(cursor_x * WFONT, cursor_y * HFONT, c, fg, bg);
		cursor_x++;
		if(cursor_x >= VGA_WIDTH / WFONT) {
			cursor_x = 0;
			cursor_y++;
		}
	}

	vga_scroll();
}

void print(const char *string) {
	nat8 fg = fg_color;
	nat8 bg = bg_color;

	for(; *string; string++) {
		if(*string == '\\') {
			string++;
			if(*string == 'x') { fg = fg_color; bg = bg_color; continue; }

			switch(*string) {
				case '0': fg=0; break; case '1': fg=1; break; case '2': fg=2; break;
				case '3': fg=3; break; case '4': fg=4; break; case '5': fg=5; break;
				case '6': fg=6; break; case '7': fg=7; break; case '8': fg=8; break;
				case '9': fg=9; break; case 'a': fg=10; break; case 'b': fg=11; break;
				case 'c': fg=12; break; case 'd': fg=13; break; case 'e': fg=14; break;
				case 'f': fg=15; break; default: fg=15; break;
			}

			string++;
			if(*string) {
				switch(*string) {
					case '0': bg=0; break; case '1': bg=1; break; case '2': bg=2; break;
					case '3': bg=3; break; case '4': bg=4; break; case '5': bg=5; break;
					case '6': bg=6; break; case '7': bg=7; break; case 'x': bg=0; break;
					default: bg=0; break;
				}
			} else string--;

			continue;
		}

		print_char(*string, fg, bg);
	}
}

static void toggle_cursor() {
	cursor_tick++;
	if(cursor_tick % 20 != 0) return;
	cursor_blink ^= 1;
    
	int px = cursor_x * WFONT;
	int py = cursor_y * HFONT;

	if(cursor_blink) vga_draw_char(px, py, '_', 0x0F, 0x00);
	else vga_draw_char(px, py, ' ', 0x0F, 0x00);
}

void do_backspace() {
	if(cursor_x == 0 && cursor_y == 0) return;

	int old_px = cursor_x * WFONT;
	int old_py = cursor_y * HFONT;
	vga_draw_char(old_px, old_py, ' ', fg_color, bg_color);

	if(cursor_x > 0) cursor_x--;
	else if(cursor_y > 0) { cursor_y--; cursor_x = (VGA_WIDTH / WFONT) - 1; }

	clear_char_area(cursor_x * WFONT, cursor_y * HFONT, bg_color);

	int px = cursor_x * WFONT;
	int py = cursor_y * HFONT;
	if(cursor_blink) vga_draw_char(px, py, '_', fg_color, bg_color);
}


void init_vga_text() {
	register_interrupt_handler(32, toggle_cursor);
}


void print_center(const char *string, nat8 bg) {
	int len = 0;
	for(const char *s = string; *s; s++) len++;

	int start_col = (VGA_WIDTH / WFONT - len) / 2;
	if(start_col < 0) start_col = 0;

	int row = cursor_y;

	if(bg != 0xFF) {
		for(int col = 0; col < VGA_WIDTH / WFONT; col++) {
			vga_draw_char(col * WFONT, row * HFONT, ' ', 0x0F, bg);
		}
	}

	for(int i = 0; string[i]; i++) {
		vga_draw_char((start_col + i) * WFONT, row * HFONT, string[i], fg_color, bg);
	}

	cursor_x = 0;
	cursor_y = row + 1;
}

void printf(const char *fmt, ...) {
	va_list args;
	va_start(args, fmt);
	char buf[32];

	for (; *fmt; fmt++) {
		if (*fmt != '%') {
			char s[2] = {*fmt, 0};
			print(s);
			continue;
		}
		fmt++;
		switch (*fmt) {
			case 's': {
				char *str = va_arg(args, char*);
				print(str);
				break;
			}
			case 'd': {
				int num = va_arg(args, int);
				char *p = buf + sizeof(buf) - 1;
				*p = 0;
				int neg = num < 0;
				if (neg) num = -num;
				do {
					*--p = '0' + (num % 10);
					num /= 10;
				} while (num);
				if (neg) *--p = '-';
				print(p);
				break;
			}
			case 'x': {
				unsigned int num = va_arg(args, unsigned int);
				char *p = buf + sizeof(buf) - 1;
				*p = 0;
				do {
					int digit = num & 0xF;
					*--p = digit < 10 ? '0' + digit : 'a' + digit - 10;
					num >>= 4;
				} while (num);
				print(p);
				break;
			}
			case 'p': {
				void *ptr = va_arg(args, void*);
				unsigned long num = (unsigned long)ptr;
				char *p = buf + sizeof(buf) - 1;
				*p = 0;
				do {
					int digit = num & 0xF;
					*--p = digit < 10 ? '0' + digit : 'a' + digit - 10;
					num >>= 4;
				} while (num);
				print("0x");
				print(p);
				break;
			}
			case 'c': {
				char c = (char)va_arg(args, int);
				char s[2] = {c, 0};
				print(s);
				break;
			}
			case '%': {
				print("%");
				break;
			}
		}
	}

	va_end(args);
}