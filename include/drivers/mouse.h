#ifndef MOUSE_H
#define MOUSE_H

#include "types.h"

void init_mouse();
extern int mouse_x;
extern int mouse_y;
bool is_left_pressed();
bool is_right_pressed();

static const nat16 cursor_mask[16] = {
    0xFF80,
    0xF000,
    0xE000,
    0xD000,
    0x8800,
    0x8400,
    0x8200,
    0x8100,
    0x8080,
    0x0040,
    0x0020,
    0x0010,
    0x0008,
    0x0004,
    0x0002,
    0x0001,
};

#endif
