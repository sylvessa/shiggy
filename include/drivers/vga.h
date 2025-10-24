#ifndef VGA_H
#define VGA_H

#include "globals.h"

#define VGA_WIDTH 640
#define VGA_HEIGHT 480
#define VGA_BYTES_PER_SCANLINE (VGA_WIDTH / 8) // 80
#define VGA_FB ((volatile nat8*)0xA0000)

void vga_draw_char(int x, int y, char c, nat8 fg, nat8 bg);
void vga_draw_text(int x, int y, const char *str, nat8 color);
void print(const char *str);
void init_vga_text();
void do_backspace();
void draw_char_pixel(int px, int py, nat8 fg, nat8 bg, int set);
void print_center(const char *string, nat8 bg);
void printf(const char *fmt, ...);
void screen_clear();
void print_char(char c, nat8 fg, nat8 bg);
void set_map_mask(nat8 mask);

#endif