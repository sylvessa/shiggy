#include "globals.h"
#include "font/main.h"

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
	60, 61, 62, 63
};

static cell_t screen_buf[MAX_ROWS][MAX_COLS];

static inline void seq_write(nat8 index, nat8 value) {
	out_byte(0x3C4, index);
	out_byte(0x3C5, value);
}

void set_map_mask(nat8 mask) {
	seq_write(0x02, mask);
}

void draw_char_pixel(int px, int py, nat8 fg, nat8 bg, int set) {
	volatile nat8 *fb = VGA_FB;
	int bytes_per_line = VGA_BYTES_PER_SCANLINE;
	nat8 color = set ? fg : bg;

	for(int plane = 0; plane < 4; plane++) {
		set_map_mask((color == 0) ? 0x0F : color);
		int byte_index = px / 8;
		int bit_index = 7 - (px % 8);
		nat8 mask = 1 << bit_index;
		nat8 val = fb[py * bytes_per_line + byte_index];

		if((color == 0) ? color & (1 << plane) : color)
			val |= mask;
		else
			val &= ~mask;

		fb[py * bytes_per_line + byte_index] = val;
	}

	set_map_mask(color);
}

void draw_pixel(int x, int y, nat8 color) {
	volatile nat8 *fb = VGA_FB;
	int bytes_per_line = VGA_BYTES_PER_SCANLINE;
	for(int plane=0; plane<4; plane++) {
		set_map_mask((color == 0) ? 0x0F : color);
		int byte = x/8;
		int bit = 7-(x%8);
		if((color == 0) ? color & (1 << plane) : color) fb[y*bytes_per_line + byte] |= (1<<bit);
		else fb[y*bytes_per_line + byte] &= ~(1<<bit);
	}
	set_map_mask(color);
}


void vga_draw_char(int x, int y, char c, nat8 fg, nat8 bg) {
	volatile nat8 *fb = VGA_FB;
	int bytes_per_line = VGA_BYTES_PER_SCANLINE;

	for(int row=0; row<HFONT; row++) {
		byte bits = (nat8)isoFont[(nat8)c][row];
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

void redraw_screen() {
	for(int y=0; y<MAX_ROWS; y++) {
		for(int x=0; x<MAX_COLS; x++) {
			cell_t *cell = &screen_buf[y][x];
			vga_draw_char(x * WFONT, y * HFONT, cell->c, cell->fg, cell->bg);
		}
	}
}

void scroll_buffer() {
	for(int y=1; y<MAX_ROWS; y++) {
		for(int x=0; x<MAX_COLS; x++) {
			screen_buf[y-1][x] = screen_buf[y][x];
		}
	}
	// clear last row
	for(int x=0; x<MAX_COLS; x++) {
		screen_buf[MAX_ROWS-1][x].c = ' ';
		screen_buf[MAX_ROWS-1][x].fg = 0x0F;
		screen_buf[MAX_ROWS-1][x].bg = 0x00;
	}
}

void print_char(char c, nat8 fg, nat8 bg) {
	if(c == '\n') {
		cursor_x = 0;
		cursor_y++;
	} else {
		if(cursor_x >= MAX_COLS) {
			cursor_x = 0;
			cursor_y++;
		}

		screen_buf[cursor_y][cursor_x].c = c;
		screen_buf[cursor_y][cursor_x].fg = fg;
		screen_buf[cursor_y][cursor_x].bg = bg;
		vga_draw_char(cursor_x * WFONT, cursor_y * HFONT, c, fg, bg);
		cursor_x++;
	}

	if(cursor_y >= MAX_ROWS) {
		scroll_buffer();
		cursor_y = MAX_ROWS - 1;
		redraw_screen();
	}
}


void screen_clear() {
	for(int y=0; y<MAX_ROWS; y++) {
		for(int x=0; x<MAX_COLS; x++) {
			screen_buf[y][x].c = ' ';
			screen_buf[y][x].fg = 0x0F;
			screen_buf[y][x].bg = 0x00;
		}
	}
	redraw_screen();
	cursor_x = 0;
	cursor_y = 0;
}

void print(const char *string) {
	nat8 fg = 0x0F;
    nat8 bg = 0x00;

	for(; *string; string++) {
		if(*string == '\\') {
			string++;
			if(*string == 'x') { fg = fg_color; bg = bg_color; continue; }

			nat8 new_fg = 15, new_bg = 0;

			switch(*string) {
				case '0': new_fg=0; break; case '1': new_fg=1; break; case '2': new_fg=2; break;
				case '3': new_fg=3; break; case '4': new_fg=4; break; case '5': new_fg=5; break;
				case '6': new_fg=6; break; case '7': new_fg=7; break; case '8': new_fg=8; break;
				case '9': new_fg=9; break; case 'a': new_fg=10; break; case 'b': new_fg=11; break;
				case 'c': new_fg=12; break; case 'd': new_fg=13; break; case 'e': new_fg=14; break;
				case 'f': new_fg=15; break; default: new_fg=15; break;
			}

			string++;
			if(*string) {
				switch(*string) {
					case '0': new_bg=0; break; case '1': new_bg=1; break; case '2': new_bg=2; break;
					case '3': new_bg=3; break; case '4': new_bg=4; break; case '5': new_bg=5; break;
					case '6': new_bg=6; break; case '7': new_bg=7; break; case 'x': new_bg=0; break;
					default: new_bg=0; break;
				}
			} else string--;

			fg = new_fg;
			bg = new_bg;
			continue;
		}

		print_char(*string, fg, bg);
	}
}


void timer_vga_callback() {
	cursor_tick++;
	if(cursor_tick % 20 != 0) return;
	if (gui_mode) return;
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

void print_center(const char *text, nat8 row_bg, ...) {
	va_list args;
	va_start(args, row_bg); // start after row_bg
	char buf[256];
	int buf_idx = 0;
	nat8 fg = fg_color;
	nat8 bg = bg_color;

	for(const char *fmt = text; *fmt; fmt++) {
		if(*fmt == '\\') {
			fmt++;
			if(*fmt == 'x') { fg = fg_color; bg = bg_color; continue; }

			nat8 new_fg = 15, new_bg = 0;
			switch(*fmt) {
				case '0': new_fg=0; break; case '1': new_fg=1; break; case '2': new_fg=2; break;
				case '3': new_fg=3; break; case '4': new_fg=4; break; case '5': new_fg=5; break;
				case '6': new_fg=6; break; case '7': new_fg=7; break; case '8': new_fg=8; break;
				case '9': new_fg=9; break; case 'a': new_fg=10; break; case 'b': new_fg=11; break;
				case 'c': new_fg=12; break; case 'd': new_fg=13; break; case 'e': new_fg=14; break;
				case 'f': new_fg=15; break; default: new_fg=15; break;
			}

			fmt++;
			if(*fmt) {
				switch(*fmt) {
					case '0': new_bg=0; break; case '1': new_bg=1; break; case '2': new_bg=2; break;
					case '3': new_bg=3; break; case '4': new_bg=4; break; case '5': new_bg=5; break;
					case '6': new_bg=6; break; case '7': new_bg=7; break; case '8': new_bg=8; break;
					case '9': new_bg=9; break; case 'a': new_bg=10; break; case 'b': new_bg=11; break;
					case 'c': new_bg=12; break; case 'd': new_bg=13; break; case 'e': new_bg=14; break;
					case 'f': new_bg=15; break;
					case 'x': new_bg=0; break;
					default: new_bg=0; break;
				}
			} else fmt--;

			fg = new_fg;
			bg = new_bg;
			continue;
		}

		if(*fmt != '%') {
			buf[buf_idx++] = *fmt;
			continue;
		}

		fmt++;
		switch(*fmt) {
			case 's': {
				char *str = va_arg(args, char*);
				for(int i=0; str[i]; i++) buf[buf_idx++] = str[i];
				break;
			}
			case 'd': {
				int num = va_arg(args,int);
				char tmp[32]; char *p = tmp + sizeof(tmp) - 1; *p=0; int neg=num<0; if(neg) num=-num;
				do {*--p='0'+(num%10); num/=10;} while(num); if(neg)*--p='-';
				for(;*p;p++) buf[buf_idx++]=*p;
				break;
			}
			case 'x': {
				unsigned int num = va_arg(args,unsigned int); char tmp[32]; char *p=tmp+sizeof(tmp)-1; *p=0;
				do { int d=num&0xF; *--p=d<10?'0'+d:'a'+d-10; num>>=4; } while(num);
				for(;*p;p++) buf[buf_idx++]=*p;
				break;
			}
			case 'c': buf[buf_idx++]=(char)va_arg(args,int); break;
			case '%': buf[buf_idx++]='%'; break;
		}
	}
	buf[buf_idx] = 0;
	va_end(args);

	int len = buf_idx;
	int start_col = (VGA_WIDTH / WFONT - len) / 2;
	if(start_col < 0) start_col = 0;

	int row = cursor_y;

	for(int col=0; col<MAX_COLS; col++)
		vga_draw_char(col*WFONT,row*HFONT,' ',0x0F,row_bg);

	for(int i=0; i<len; i++)
		vga_draw_char((start_col+i)*WFONT,row*HFONT,buf[i],fg,row_bg);

	cursor_x = 0;
	cursor_y = row+1;
}


void printf(const char *fmt, ...) {
	va_list args;
	va_start(args, fmt);
	char buf[32];
	nat8 fg = fg_color;
	nat8 bg = bg_color;

	for (; *fmt; fmt++) {
		if(*fmt == '\\') {
			fmt++;
			if(*fmt == 'x') { fg = fg_color; bg = bg_color; continue; }

			nat8 new_fg = 15, new_bg = 0;
			switch(*fmt) {
				case '0': new_fg=0; break; case '1': new_fg=1; break; case '2': new_fg=2; break;
				case '3': new_fg=3; break; case '4': new_fg=4; break; case '5': new_fg=5; break;
				case '6': new_fg=6; break; case '7': new_fg=7; break; case '8': new_fg=8; break;
				case '9': new_fg=9; break; case 'a': new_fg=10; break; case 'b': new_fg=11; break;
				case 'c': new_fg=12; break; case 'd': new_fg=13; break; case 'e': new_fg=14; break;
				case 'f': new_fg=15; break; default: new_fg=15; break;
			}

			fmt++;
			if(*fmt) {
				switch(*fmt) {
					case '0': new_bg=0; break; case '1': new_bg=1; break; case '2': new_bg=2; break;
					case '3': new_bg=3; break; case '4': new_bg=4; break; case '5': new_bg=5; break;
					case '6': new_bg=6; break; case '7': new_bg=7; break; case '8': new_bg=8; break;
					case '9': new_bg=9; break; case 'a': new_bg=10; break; case 'b': new_bg=11; break;
					case 'c': new_bg=12; break; case 'd': new_bg=13; break; case 'e': new_bg=14; break;
					case 'f': new_bg=15; break;
					case 'x': new_bg=0; break;
					default: new_bg=0; break;
				}
			} else fmt--;

			fg = new_fg;
			bg = new_bg;
			continue;
		}

		if (*fmt != '%') {
			print_char(*fmt, fg, bg);
			continue;
		}

		fmt++;
		switch (*fmt) {
			case 's': {
				char *str = va_arg(args, char*);
				for(int i=0; str[i]; i++) print_char(str[i], fg, bg);
				break;
			}
			case 'd': {
				int num = va_arg(args, int);
				char *p = buf + sizeof(buf) - 1;
				*p = 0;
				int neg = num < 0;
				if(neg) num = -num;
				do {
					*--p = '0' + (num % 10);
					num /= 10;
				} while(num);
				if(neg) *--p = '-';
				for(char *c=p; *c; c++) print_char(*c, fg, bg);
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
				} while(num);
				for(char *c=p; *c; c++) print_char(*c, fg, bg);
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
				} while(num);
				print("0x");
				for(char *c=p; *c; c++) print_char(*c, fg, bg);
				break;
			}
			case 'c': {
				char c = (char)va_arg(args, int);
				print_char(c, fg, bg);
				break;
			}
			case '%': {
				print_char('%', fg, bg);
				break;
			}
		}
	}

	va_end(args);
}

void printf_at(int col, int row, const char *fmt, ...) {
	va_list args;
	va_start(args, fmt);
	char buf[32];
	nat8 fg = fg_color;
	nat8 bg = bg_color;

	for(; *fmt; fmt++) {
		if(*fmt == '\\') {
			fmt++;
			if(*fmt == 'x') { fg = fg_color; bg = bg_color; continue; }

			nat8 new_fg = 15, new_bg = 0;
			switch(*fmt) {
				case '0': new_fg=0; break; case '1': new_fg=1; break; case '2': new_fg=2; break;
				case '3': new_fg=3; break; case '4': new_fg=4; break; case '5': new_fg=5; break;
				case '6': new_fg=6; break; case '7': new_fg=7; break; case '8': new_fg=8; break;
				case '9': new_fg=9; break; case 'a': new_fg=10; break; case 'b': new_fg=11; break;
				case 'c': new_fg=12; break; case 'd': new_fg=13; break; case 'e': new_fg=14; break;
				case 'f': new_fg=15; break; default: new_fg=15; break;
			}

			fmt++;
			if(*fmt) {
				switch(*fmt) {
					case '0': new_bg=0; break; case '1': new_bg=1; break; case '2': new_bg=2; break;
					case '3': new_bg=3; break; case '4': new_bg=4; break; case '5': new_bg=5; break;
					case '6': new_bg=6; break; case '7': new_bg=7; break; case '8': new_bg=8; break;
					case '9': new_bg=9; break; case 'a': new_bg=10; break; case 'b': new_bg=11; break;
					case 'c': new_bg=12; break; case 'd': new_bg=13; break; case 'e': new_bg=14; break;
					case 'f': new_bg=15; break;
					case 'x': new_bg=0; break;
					default: new_bg=0; break;
				}
			} else fmt--;

			fg = new_fg;
			bg = new_bg;
			continue;
		}

		if(*fmt != '%') {
			vga_draw_char(col * WFONT, row * HFONT, *fmt, fg, bg);
			col++;
			continue;
		}

		fmt++;
		switch(*fmt) {
			case 's': {
				char *str = va_arg(args, char*);
				for(int i=0; str[i]; i++) { vga_draw_char(col*WFONT,row*HFONT,str[i],fg,bg); col++; }
				break;
			}
			case 'd': {
				int num = va_arg(args,int);
				char *p=buf+sizeof(buf)-1; *p=0; int neg=num<0; if(neg) num=-num;
				do{*--p='0'+(num%10); num/=10;}while(num); if(neg)*--p='-';
				for(char *c=p;*c;c++){vga_draw_char(col*WFONT,row*HFONT,*c,fg,bg); col++;}
				break;
			}
			case 'x': {
				unsigned int num=va_arg(args,unsigned int); char *p=buf+sizeof(buf)-1; *p=0;
				do{int d=num&0xF; *--p=d<10?'0'+d:'a'+d-10; num>>=4;}while(num);
				for(char *c=p;*c;c++){vga_draw_char(col*WFONT,row*HFONT,*c,fg,bg); col++;}
				break;
			}
			case 'c': { char c=(char)va_arg(args,int); vga_draw_char(col*WFONT,row*HFONT,c,fg,bg); col++; break; }
			case '%': { vga_draw_char(col*WFONT,row*HFONT,'%',fg,bg); col++; break; }
		}
	}

	va_end(args);
}

void do_clear_cursor() {
	int px = cursor_x * WFONT;
	int py = cursor_y * HFONT;

	clear_char_area(px, py, bg_color);
	cursor_blink = 0;
}

void set_bg_color(nat8 color) {
	bg_color = color;

	for(int y=0; y<MAX_ROWS; y++) {
		for(int x=0; x<MAX_COLS; x++) {
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