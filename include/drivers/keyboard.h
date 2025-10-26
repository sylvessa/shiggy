#ifndef KEYBOARD_H
#define KEYBOARD_H

#define BACKSPACE 0x0E
#define ENTER 0x1C
#define SC_MAX 114
#define LSHIFT 0x2A
#define RSHIFT 0x36
#define CAPSLOCK 0x3A
#define INITIAL_REPEAT_DELAY 30
#define REPEAT_RATE 5

void init_keyboard();
void sconf(char* buffer);

#endif