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

int handle_scrolling(int cursor_offset)
{
    if (cursor_offset < MAX_ROWS * MAX_COLS * 2) return cursor_offset;

    for (nat32 i = 1; i < MAX_ROWS; i++)
        memcpy((byte *)(VIDEO_ADDRESS + GET_SCREEN_OFFSET(0, i)),
                    (byte *)(VIDEO_ADDRESS + GET_SCREEN_OFFSET(0, i - 1)),
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
	char color = WHITE_ON_BLACK;
	for (; *string; string++) {
        
		if (*string == '\\') {
			string++;
			switch (*string) {
				case '0': color = 0x00; break; // black
				case '1': color = 0x01; break; // blue
				case '2': color = 0x02; break; // green
				case '3': color = 0x03; break; // cyan
				case '4': color = 0x04; break; // red
				case '5': color = 0x05; break; // magenta
				case '6': color = 0x06; break; // brown
				case '7': color = 0x07; break; // light gray
				case '8': color = 0x08; break; // dark gray
				case '9': color = 0x09; break; // light blue
				case 'a': color = 0x0a; break; // light green
				case 'b': color = 0x0b; break; // light cyan
				case 'c': color = 0x0c; break; // light red
				case 'd': color = 0x0d; break; // light magenta
				case 'e': color = 0x0e; break; // yellow
				case 'f': color = 0x0f; break; // white
				default: color = WHITE_ON_BLACK; break;
			}

			continue;
		}

		print_char(*string, -1, -1, color);
	}
}
