#include "font/main.h"
#include "globals.h"

// todo: combine all the formatting for print and printf's into a single func whenever i make a new print that does something unique

#define MAX_COLS (VGA_WIDTH / WFONT)
#define MAX_ROWS (VGA_HEIGHT / HFONT)

static int cursor_x = 0;
static int cursor_y = 0;
static nat8 fg_color = 0x0F;
static nat8 bg_color = 0x00;
static int cursor_blink = 0;
static nat32 cursor_tick = 0;

typedef struct {
	char c;
	nat8 fg;
	nat8 bg;
} cell_t;

static nat8 ega_to_dac[16] = {
	0, 1, 2, 3,
	4, 5, 20, 7,
	56, 57, 58, 59,
	60, 61, 62, 63};

static cell_t screen_buf[MAX_ROWS][MAX_COLS];

static inline void seq_write(nat8 index, nat8 value) {
	out_byte(0x3C4, index);
	out_byte(0x3C5, value);
}

void set_map_mask(nat8 mask) {
	seq_write(0x02, mask);
}

static nat8 hex_digit(char c) {
	if (c >= '0' && c <= '9') return c - '0';
	if (c >= 'a' && c <= 'f') return 10 + (c - 'a');
	if (c >= 'A' && c <= 'F') return 10 + (c - 'A');
	return 0;
}

static int format_to_buffer(formatted_char* out, int out_sz, const char* fmt, va_list args, nat8* end_fg, nat8* end_bg, bool enable_formatting) {
	int idx = 0;
	nat8 fg = fg_color;
	nat8 bg = bg_color;
	for (; *fmt && idx < out_sz - 1; fmt++) {
		if (*fmt == '\\') {
			fmt++;
			if (!*fmt) break;
			if (*fmt == 'x') {
				fg = fg_color;
				bg = bg_color;
				continue;
			}

			nat8 new_fg = hex_digit(*fmt);
			fmt++;
			nat8 new_bg = *fmt ? hex_digit(*fmt) : 0;
			if (!*fmt) fmt--;
			fg = new_fg;
			bg = new_bg;
			continue;
		}

		if (*fmt == '%' && !enable_formatting) {
			out[idx].ch = '%';
			out[idx].fg = fg;
			out[idx].bg = bg;
			idx++;
			continue;
		}

		if (*fmt != '%') {
			out[idx].ch = *fmt;
			out[idx].fg = fg;
			out[idx].bg = bg;
			idx++;
			continue;
		}
		fmt++;
		if (!*fmt) break;
		switch (*fmt) {
			case 's': {
				char* s = va_arg(args, char*);
				for (int i = 0; s[i] && idx < out_sz - 1; i++) {
					out[idx].ch = s[i];
					out[idx].fg = fg;
					out[idx].bg = bg;
					idx++;
				}
				break;
			}
			case 'd': {
				int n = va_arg(args, int);
				char tmp[32];
				char* p = tmp + sizeof(tmp) - 1;
				*p = 0;
				int neg = n < 0;
				if (neg) n = -n;
				do {
					*--p = '0' + (n % 10);
					n /= 10;
				} while (n);
				if (neg) *--p = '-';
				for (; *p && idx < out_sz - 1; p++) {
					out[idx].ch = *p;
					out[idx].fg = fg;
					out[idx].bg = bg;
					idx++;
				}
				break;
			}
			case 'x': {
				unsigned int n = va_arg(args, unsigned int);
				char tmp[32];
				char* p = tmp + sizeof(tmp) - 1;
				*p = 0;
				do {
					int d = n & 0xF;
					*--p = d < 10 ? '0' + d : 'a' + d - 10;
					n >>= 4;
				} while (n);
				for (; *p && idx < out_sz - 1; p++) {
					out[idx].ch = *p;
					out[idx].fg = fg;
					out[idx].bg = bg;
					idx++;
				}
				break;
			}
			case 'f': {
				double num = va_arg(args, double);
				int int_part = (int)num;
				double frac_part = num - int_part;
				if (frac_part < 0) frac_part = -frac_part;

				int frac = (int)(frac_part * 100);

				char tmp[32];
				char* p = tmp + sizeof(tmp) - 1;
				*p = 0;

				int neg = int_part < 0;
				if (neg) int_part = -int_part;

				do {
					*--p = '0' + (int_part % 10);
					int_part /= 10;
				} while (int_part);

				if (neg) *--p = '-';

				for (; *p && idx < out_sz - 1; p++) {
					out[idx].ch = *p;
					out[idx].fg = fg;
					out[idx].bg = bg;
					idx++;
				}

				if (idx < out_sz - 1) {
					out[idx].ch = '.';
					out[idx].fg = fg;
					out[idx].bg = bg;
					idx++;
				}

				char frac_buf[3];
				frac_buf[0] = '0' + ((frac / 10) % 10);
				frac_buf[1] = '0' + (frac % 10);
				frac_buf[2] = 0;

				for (int i = 0; frac_buf[i] && idx < out_sz - 1; i++) {
					out[idx].ch = frac_buf[i];
					out[idx].fg = fg;
					out[idx].bg = bg;
					idx++;
				}

				break;
			}
			case 'p': {
				void* ptr = va_arg(args, void*);
				unsigned long n = (unsigned long)ptr;
				char tmp[32];
				char* p = tmp + sizeof(tmp) - 1;
				*p = 0;
				do {
					int d = n & 0xF;
					*--p = d < 10 ? '0' + d : 'A' + d - 10;
					n >>= 4;
				} while (n);
				for (; *p && idx < out_sz - 1; p++) {
					out[idx].ch = *p;
					out[idx].fg = fg;
					out[idx].bg = bg;
					idx++;
				}
				break;
			}
			case 'c': {
				int ci = va_arg(args, int);
				out[idx].ch = (char)ci;
				out[idx].fg = fg;
				out[idx].bg = bg;
				idx++;
				break;
			}
			case '%': {
				out[idx].ch = '%';
				out[idx].fg = fg;
				out[idx].bg = bg;
				idx++;
				break;
			}
		}
	}
	if (end_fg) *end_fg = fg;
	if (end_bg) *end_bg = bg;
	if (idx >= out_sz) idx = out_sz - 1;
	out[idx].ch = 0;
	return idx;
}

void draw_char_pixel(int px, int py, nat8 fg, nat8 bg, int set) {
	volatile nat8* fb = VGA_FB;
	int bytes_per_line = VGA_BYTES_PER_SCANLINE;
	nat8 color = set ? fg : bg;

	for (int plane = 0; plane < 4; plane++) {
		set_map_mask((color == 0) ? 0x0F : color);
		int byte_index = px / 8;
		int bit_index = 7 - (px % 8);
		nat8 mask = 1 << bit_index;
		nat8 val = fb[py * bytes_per_line + byte_index];

		if ((color == 0) ? color & (1 << plane) : color)
			val |= mask;
		else
			val &= ~mask;

		fb[py * bytes_per_line + byte_index] = val;
	}

	set_map_mask(color);
}

void draw_pixel(int x, int y, nat8 color) {
	volatile nat8* fb = VGA_FB;
	int bytes_per_line = VGA_BYTES_PER_SCANLINE;
	for (int plane = 0; plane < 4; plane++) {
		set_map_mask((color == 0) ? 0x0F : color);
		int byte = x / 8;
		int bit = 7 - (x % 8);
		if ((color == 0) ? color & (1 << plane) : color)
			fb[y * bytes_per_line + byte] |= (1 << bit);
		else
			fb[y * bytes_per_line + byte] &= ~(1 << bit);
	}
	set_map_mask(color);
}

void vga_draw_char(int x, int y, char c, nat8 fg, nat8 bg) {
	volatile nat8* fb = VGA_FB;
	int bytes_per_line = VGA_BYTES_PER_SCANLINE;

	for (int row = 0; row < HFONT; row++) {
		byte bits = (nat8)isoFont[(nat8)c][row];
		int base = x / 8;

		for (int plane = 0; plane < 4; plane++) {
			set_map_mask(1 << plane);
			nat8 byte_val = 0;
			for (int bit = 0; bit < 8; bit++) {
				int mask = 1 << (7 - bit);
				int pixel_on = (bits & mask) != 0;
				if (pixel_on)
					byte_val |= ((fg >> plane) & 1) << (7 - bit);
				else
					byte_val |= ((bg >> plane) & 1) << (7 - bit);
			}
			fb[(y + row) * bytes_per_line + base] = byte_val;
		}
	}
	set_map_mask(0x0F);
}

static void clear_char_area(int cx, int cy, nat8 color) {
	for (int y = 0; y < HFONT; y++) {
		for (int x = 0; x < WFONT; x++) {
			draw_char_pixel(cx + x, cy + y, color, color, 0);
		}
	}
}

void redraw_screen() {
	for (int y = 0; y < MAX_ROWS; y++) {
		for (int x = 0; x < MAX_COLS; x++) {
			cell_t* cell = &screen_buf[y][x];
			vga_draw_char(x * WFONT, y * HFONT, cell->c, cell->fg, cell->bg);
		}
	}
}

void scroll_buffer() {
	for (int y = 1; y < MAX_ROWS; y++) {
		for (int x = 0; x < MAX_COLS; x++) {
			screen_buf[y - 1][x] = screen_buf[y][x];
		}
	}
	// clear last row
	for (int x = 0; x < MAX_COLS; x++) {
		screen_buf[MAX_ROWS - 1][x].c = ' ';
		screen_buf[MAX_ROWS - 1][x].fg = 0x0F;
		screen_buf[MAX_ROWS - 1][x].bg = 0x00;
	}
}

void print_char(char c, nat8 fg, nat8 bg) {
	if (c == '\n') {
		cursor_x = 0;
		cursor_y++;
	} else {
		if (cursor_x >= MAX_COLS) {
			cursor_x = 0;
			cursor_y++;
		}

		screen_buf[cursor_y][cursor_x].c = c;
		screen_buf[cursor_y][cursor_x].fg = fg;
		screen_buf[cursor_y][cursor_x].bg = bg;
		vga_draw_char(cursor_x * WFONT, cursor_y * HFONT, c, fg, bg);
		cursor_x++;
	}

	if (cursor_y >= MAX_ROWS) {
		scroll_buffer();
		cursor_y = MAX_ROWS - 1;
		redraw_screen();
	}
}

void screen_clear() {
	for (int y = 0; y < MAX_ROWS; y++) {
		for (int x = 0; x < MAX_COLS; x++) {
			screen_buf[y][x].c = ' ';
			screen_buf[y][x].fg = 0x0F;
			screen_buf[y][x].bg = 0x00;
		}
	}
	redraw_screen();
	cursor_x = 0;
	cursor_y = 0;
}

void print(const char* string) {
	nat8 fg = fg_color;
	nat8 bg = bg_color;

	formatted_char buf[1024];
	int len = format_to_buffer(buf, 1024, string, NULL, &fg, &bg, false);

	for (int i = 0; i < len; i++) {
		print_char(buf[i].ch, buf[i].fg, buf[i].bg);
	}
}

void timer_vga_callback() {
	cursor_tick++;
	if (cursor_tick % 20 != 0) return;
	if (gui_mode) return;
	cursor_blink ^= 1;

	int px = cursor_x * WFONT;
	int py = cursor_y * HFONT;

	if (cursor_blink)
		vga_draw_char(px, py, '_', 0x0F, 0x00);
	else
		vga_draw_char(px, py, ' ', 0x0F, 0x00);
}

void do_backspace() {
	if (cursor_x == 0 && cursor_y == 0) return;

	int old_px = cursor_x * WFONT;
	int old_py = cursor_y * HFONT;
	vga_draw_char(old_px, old_py, ' ', fg_color, bg_color);

	if (cursor_x > 0)
		cursor_x--;
	else if (cursor_y > 0) {
		cursor_y--;
		cursor_x = (VGA_WIDTH / WFONT) - 1;
	}

	clear_char_area(cursor_x * WFONT, cursor_y * HFONT, bg_color);

	int px = cursor_x * WFONT;
	int py = cursor_y * HFONT;
	if (cursor_blink) vga_draw_char(px, py, '_', fg_color, bg_color);
}

void print_center(const char* fmt, nat8 row_bg, ...) {
	va_list args;
	va_start(args, row_bg);
	formatted_char buf[1024];
	int len = format_to_buffer(buf, sizeof(buf) / sizeof(buf[0]), fmt, args, NULL, NULL, true);
	va_end(args);

	int start_col = (VGA_WIDTH / WFONT - len) / 2;
	if (start_col < 0) start_col = 0;

	int row = cursor_y;
	for (int col = 0; col < MAX_COLS; col++)
		vga_draw_char(col * WFONT, row * HFONT, ' ', 0x0F, row_bg);

	for (int i = 0; i < len; i++)
		vga_draw_char((start_col + i) * WFONT, row * HFONT, buf[i].ch, buf[i].fg, row_bg);

	cursor_x = 0;
	cursor_y = row + 1;
}

void printf(const char* fmt, ...) {
	va_list args;

	va_start(args, fmt);
	formatted_char buf[1024];
	int len = format_to_buffer(buf, sizeof(buf) / sizeof(buf[0]), fmt, args, NULL, NULL, true);
	va_end(args);

	for (int i = 0; i < len; i++) {
		print_char(buf[i].ch, buf[i].fg, buf[i].bg);
	}
}

void printf_at(int col, int row, const char* fmt, ...) {
	va_list args;
	va_start(args, fmt);
	formatted_char buf[1024];
	int len = format_to_buffer(buf, sizeof(buf) / sizeof(buf[0]), fmt, args, NULL, NULL, true);
	va_end(args);

	for (int i = 0; i < len; i++) {
		vga_draw_char((col + i) * WFONT, row * HFONT, buf[i].ch, buf[i].fg, buf[i].bg);
	}
}

void do_clear_cursor() {
	int px = cursor_x * WFONT;
	int py = cursor_y * HFONT;

	clear_char_area(px, py, bg_color);
	cursor_blink = 0;
}

void set_bg_color(nat8 color) {
	bg_color = color;

	for (int y = 0; y < MAX_ROWS; y++) {
		for (int x = 0; x < MAX_COLS; x++) {
			screen_buf[y][x].bg = color;
			vga_draw_char(x * WFONT, y * HFONT, screen_buf[y][x].c, screen_buf[y][x].fg, color);
		}
	}
}

void set_color(nat8 color, nat8 r255, nat8 g255, nat8 b255) {
	nat8 r = r255 >> 2;
	nat8 g = g255 >> 2;
	nat8 b = b255 >> 2;

	nat8 dac = ega_to_dac[color];

	out_byte(0x3C8, dac);
	out_byte(0x3C9, r);
	out_byte(0x3C9, g);
	out_byte(0x3C9, b);
}