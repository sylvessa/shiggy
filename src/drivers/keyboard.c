#include "globals.h"
#include "drivers/keyboard.h"

static char input_buffer[256];
static nat32 input_size = 0;
static bool ready = false;
static bool shift_pressed = false;
static bool caps_lock_on = false;

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

void do_backspace() {
	int offset = get_cursor_offset();
	if (offset <= 0) return;
	offset -= 2;
	byte *vid = (byte *)(VIDEO_ADDRESS + offset);
	vid[0] = ' ';
	vid[1] = WHITE_ON_BLACK;
	set_cursor_offset(offset);
}

void keyboard_callback() {
	nat8 scancode = in_byte(0x60);
	bool released = (scancode & 0x80) != 0;
	nat8 code = scancode & 0x7F;

	if (code > SC_MAX) return;

	if (code == LSHIFT || code == RSHIFT) {
		shift_pressed = !released;
		return;
	}

    // if (code == CAPSLOCK && !released) {
	// 	caps_lock_on = !caps_lock_on;
	// 	return;
	// }

	if (released) return;

	switch (code) {
		case BACKSPACE:
			if (input_size > 0) {
				input_size--;
				do_backspace();
			}
			break;
		case ENTER:
			input_buffer[input_size] = '\0';
			print("\n");
			ready = true;
			break;
		default: {
			char letter = (shift_pressed || caps_lock_on) ? sc_ascii_shift[code] : sc_ascii[code];
			if (letter != '?' && input_size < (sizeof(input_buffer) - 1)) {
				input_buffer[input_size++] = letter;
				char str[2] = {letter, '\0'};
				print(str);
			}
			break;
		}
	}
}

void init_keyboard() {
	register_interrupt_handler(33, keyboard_callback); // irq1
}

void sconf(char *buffer) {
	while (!ready);
	strcpy(input_buffer, buffer);
	input_size = 0;
	ready = false;
}
