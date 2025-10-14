#include "globals.h"
#include "drivers/keyboard.h"

static char input_buffer[256];
static nat32 input_size = 0;
static bool ready = false;
static bool shift_pressed = false;

const char sc_ascii[] = {'?', '?', '1', '2', '3', '4', '5', '6',
                         '7', '8', '9', '0', '-', '=', '?', '?', 'q', 'w', 'e', 'r', 't', 'y',
                         'u', 'i', 'o', 'p', '[', ']', '?', '?', 'a', 's', 'd', 'f', 'g',
                         'h', 'j', 'k', 'l', ';', '\'', '`', '?', '\\', 'z', 'x', 'c', 'v',
                         'b', 'n', 'm', ',', '.', '/', '?', '?', '?', ' '};

const char sc_ascii_shift[] = {'?', '?', '!', '@', '#', '$', '%', '^',
                               '&', '*', '(', ')', '_', '+', '?', '?', 'Q', 'W', 'E', 'R', 'T', 'Y',
                               'U', 'I', 'O', 'P', '{', '}', '?', '?', 'A', 'S', 'D', 'F', 'G',
                               'H', 'J', 'K', 'L', ':', '"', '~', '?', '|', 'Z', 'X', 'C', 'V',
                               'B', 'N', 'M', '<', '>', '?', '?', '?', ' '};

void keyboard_callback(registers_t regs) {
    nat8 scancode = in_byte(0x60); // PIC leaves scancode in port 0x60
	bool released = scancode & 0x80;

    if (scancode > SC_MAX) return;

    if (scancode == LSHIFT || scancode == RSHIFT) {
		shift_pressed = !released;
		return;
	}

	if (released) return;

    switch (scancode) {
        case BACKSPACE:
            input_size = input_size - 1;
            break;
        case ENTER:
            input_buffer[input_size] = '\0';
            print("\n");
            ready = true;
            break;
        default: {
            char letter = shift_pressed ? sc_ascii_shift[(int)scancode] : sc_ascii[(int)scancode];
            input_buffer[input_size++] = letter;
            char str[2] = {letter, '\0'};
            print(str);
            break;
        }
    }
}

void init_keyboard() {
    register_interrupt_handler(33, keyboard_callback); // 33 = IRQ1
}

void sconf(char *buffer) {
    while (!ready);
    strcpy(input_buffer, buffer);
    input_size = 0;
    ready = false;
}