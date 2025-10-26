#ifndef GRAPHICS_MAIN_H
#define GRAPHICS_MAIN_H

#include "globals.h"

void gfx_draw_pixel(int x, int y, nat8 color);
void gfx_draw_line(int x0, int y0, int x1, int y1, nat8 color, int thickness);

#endif