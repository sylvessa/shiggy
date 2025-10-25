#include "globals.h"
#include "drivers/keyboard.h"
#include "apps/gfx.h"

static char input_buffer[256];
static nat32 input_size = 0;
static bool ready = false;
static bool shift_pressed = false;
static bool caps_lock_on = false;
static bool extended = false;

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


void keyboard_callback() {
	nat8 scancode = in_byte(0x60);
	bool released = (scancode & 0x80) != 0;
	nat8 code = scancode & 0x7F;

	if (scancode == 0xE0) { 
		extended = true;
		return;
	}

	if (extended) {
		float move_amount = 1.0f;

		switch (code) {
			case 0x48: // up
				if (!released && !is_mesh_empty(&cube)) translate_mesh(&cube, 0, 0, -move_amount);
				break;
			case 0x50: // down
				if (!released && !is_mesh_empty(&cube)) translate_mesh(&cube, 0, 0, move_amount);
				break;
			case 0x4B: // left
				if (!released && !is_mesh_empty(&cube)) translate_mesh(&cube, -move_amount, 0, 0);
				break;
			case 0x4D: // right
				if (!released && !is_mesh_empty(&cube)) translate_mesh(&cube, move_amount, 0, 0);
				break;
		}
		extended = false;
		return;
	}


	if (code > SC_MAX) return;

	if (code == LSHIFT || code == RSHIFT) {
		shift_pressed = !released;
		return;
	}

	if (released) return;
	if (gui_mode) return;

	switch (code) {
		case BACKSPACE:
			if (!released && !is_mesh_empty(&cube)) translate_mesh(&cube, 0, 1.0f, 0);
			if (input_size > 0) {
				input_size--;
				do_backspace();
			}
			break;
		case ENTER:
			input_buffer[input_size] = '\0';
			do_clear_cursor();
			print("\n");
			ready = true;
			break;
		case 0x39:
			if (!released && !is_mesh_empty(&cube)) translate_mesh(&cube, 0, -1.0f, 0);
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
	strcpy(buffer, input_buffer);
	input_size = 0;
	ready = false;
}
