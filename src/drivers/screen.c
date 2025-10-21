#include "globals.h"
#include "drivers/screen.h"

int get_cursor_offset() {
    out_byte(REG_SCREEN_CTRL, 14);
    int offset = in_byte(REG_SCREEN_DATA) << 8;
    out_byte(REG_SCREEN_CTRL, 15);
    offset += in_byte(REG_SCREEN_DATA);
    return offset * 2;
}

void set_cursor_offset(int offset) {
    offset /= 2;
    out_byte(REG_SCREEN_CTRL, 14);
    out_byte(REG_SCREEN_DATA, (byte)(offset >> 8));
    out_byte(REG_SCREEN_CTRL, 15);
    out_byte(REG_SCREEN_DATA, (byte)(offset & 0xff));
}

int handle_scrolling(int cursor_offset) {
    if (cursor_offset < MAX_ROWS * MAX_COLS * 2) return cursor_offset;

    for (nat32 i = 1; i < MAX_ROWS; i++)
        memcpy((byte *)(VIDEO_ADDRESS + GET_SCREEN_OFFSET(0, i - 1)), (byte *)(VIDEO_ADDRESS + GET_SCREEN_OFFSET(0, i)),
                    MAX_COLS * 2);

    byte *last_line = (byte *)(VIDEO_ADDRESS + GET_SCREEN_OFFSET(0, MAX_ROWS - 1));

    for (nat32 i = 0; i < MAX_COLS * 2; i++)
        last_line[i] = 0;
    
    cursor_offset -= 2 * MAX_COLS;
    return cursor_offset;
}

void screen_set_cursor(uint8_t x, uint8_t y) {
	uint16_t pos = y * MAX_COLS + x;
	out_byte(REG_SCREEN_CTRL, 0x0F);
	out_byte(REG_SCREEN_DATA, (uint8_t)(pos & 0xFF));
	out_byte(REG_SCREEN_CTRL, 0x0E);
	out_byte(REG_SCREEN_DATA, (uint8_t)((pos >> 8) & 0xFF));
}

void screen_clear() {
    uint16_t *vga = (uint16_t*)VIDEO_ADDRESS;
	uint16_t blank = 0x0F20;

	for (int i = 0; i < MAX_COLS * MAX_ROWS; i++)
		vga[i] = blank;
        
	screen_set_cursor(0, 0);
}

void print_char(char character, int col, int row, char attribute_byte) {
    unsigned char *vidmem = (unsigned char *)VIDEO_ADDRESS;

    if (!attribute_byte) attribute_byte = WHITE_ON_BLACK;

    int offset;
    if (col >= 0 && row >= 0)
        offset = GET_SCREEN_OFFSET(col, row);
    else
        offset = get_cursor_offset();

    if (character == '\n') {
        int rows = offset / (2 * MAX_COLS);
        offset = GET_SCREEN_OFFSET(79, rows);
    } else {
        vidmem[offset] = character;
        vidmem[offset + 1] = attribute_byte;
    }

    offset += 2;

    offset = handle_scrolling(offset);

    set_cursor_offset(offset);
}

void print(char *string) {
	uint8_t fg = 0x0f; // white
	uint8_t bg = 0x00; // black
	uint8_t color = (bg << 4) | fg;

	for (; *string; string++) {
		if (*string == '\\') {
			string++;
			if (*string == 'x') { // reset
				fg = 0x0f;
				bg = 0x00;
				color = (bg << 4) | fg;
				continue;
			}

			// parse foreground
			switch (*string) {
				case '0': fg = 0x0; break; // black
				case '1': fg = 0x1; break; // blue
				case '2': fg = 0x2; break; // green
				case '3': fg = 0x3; break; // cyan
				case '4': fg = 0x4; break; // red
				case '5': fg = 0x5; break; // magenta
				case '6': fg = 0x6; break; // brown
				case '7': fg = 0x7; break; // light gray
				case '8': fg = 0x8; break; // dark gray
				case '9': fg = 0x9; break; // light blue
				case 'a': fg = 0xa; break; // light green
				case 'b': fg = 0xb; break; // light cyan
				case 'c': fg = 0xc; break; // light red
				case 'd': fg = 0xd; break; // light magenta
				case 'e': fg = 0xe; break; // yellow
				case 'f': fg = 0xf; break; // white
				default: fg = 0x0f; break;
			}

			// optional background
			string++;
			if (*string) {
				switch (*string) {
					case '0': bg = 0x0; break; // black
					case '1': bg = 0x1; break; // blue
					case '2': bg = 0x2; break; // green
					case '3': bg = 0x3; break; // cyan
					case '4': bg = 0x4; break; // red
					case '5': bg = 0x5; break; // magenta
					case '6': bg = 0x6; break; // brown
					case '7': bg = 0x7; break; // light gray
                    case 'x': bg = 0x0; break;
					default: bg = 0x0; break; // default black
				}
			} else {
				string--; // no bg provided, step back
			}

			color = (bg << 4) | fg;
			continue;
		}

		print_char(*string, -1, -1, color);
	}
}

void print_centered(char *string, uint8_t bg) {
	int len = 0;
	for (char *s = string; *s; s++) len++;

	int start_col = (MAX_COLS - len) / 2;
	if (start_col < 0) start_col = 0;

	int row = get_cursor_offset() / (2 * MAX_COLS);

	if (bg != 0xFF) {
		for (int col = 0; col < MAX_COLS; col++) {
			uint8_t color = (bg << 4) | 0x0f;
			print_char(' ', col, row, color);
		}
	}

	for (int i = 0; string[i]; i++) {
		uint8_t color = (bg << 4) | 0x0f;
		print_char(string[i], start_col + i, row, color);
	}

	set_cursor_offset(GET_SCREEN_OFFSET(0, row + 1));
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
					int digit = num & 0xf;
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
					int digit = num & 0xf;
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
