#pragma once

#define MAX_ROWS 25
#define MAX_COLS 80
#define VIDEO_ADDRESS 0xB8000

// I/O PORTS
#define REG_SCREEN_CTRL 0x3D4
#define REG_SCREEN_DATA 0x3D5

#define WHITE_ON_BLACK 0x0f // def

#define GET_SCREEN_OFFSET(COL, ROW) (2 * ((ROW) * MAX_COLS + (COL)))
#define GET_OFFSET_ROW(OFFSET) (OFFSET / (2 * MAX_COLS))
#define GET_OFFSET_COL(OFFSET) ((OFFSET - (GET_OFFSET_ROW(OFFSET) * 2 * MAX_COLS)) / 2)

void screen_clear();
void screen_set_cursor(uint8_t x, uint8_t y);
void print(char *string);
void set_cursor_offset(int offset);
int get_cursor_offset();
void print_char(char character, int col, int row, char attribute_byte);

