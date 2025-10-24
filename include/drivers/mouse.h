#ifndef MOUSE_H
#define MOUSE_H

#include "types.h"

void init_mouse();
extern int mouse_x;
extern int mouse_y;
bool is_left_pressed();
bool is_right_pressed();

#endif
