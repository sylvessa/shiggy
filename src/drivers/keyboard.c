#include "drivers/keyboard.h"
#include "apps/base.h"
#include "apps/gfx.h"
#include "globals.h"

static char input_buffer[256];
static nat32 input_size = 0;
static bool ready = false;
static bool shift_pressed = false;
static bool caps_lock_on = false;
static bool extended = false;

const char sc_ascii[] = {'?', '?', '1', '2', '3', '4', '5', '6', '7', '8', '9',	 '0', '-', '=',	 '?',
						 '?', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p',	 '[', ']', '?',	 '?',
						 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', '?', '\\', 'z',
						 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', '?', '?',	 '?', ' '};

const char sc_ascii_shift[] = {'?', '?', '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '?',
							   '?', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '?', '?',
							   'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~', '?', '|', 'Z',
							   'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', '?', '?', ' '};

static char to_lower(char c) {
	if (c >= 'A' && c <= 'Z')
		return c + 32;
	return c;
}

static bool prefix_case(const char* s, const char* pre) {
	while (*pre) {
		if (to_lower(*s) != to_lower(*pre))
			return false;
		s++;
		pre++;
	}
	return true;
}

static void echo_char(char c) {
	char s[2] = {c, '\0'};
	print(s);
}

#define TAB_MATCH_MAX 64

static char tab_matches[TAB_MATCH_MAX][256];
static nat32 tab_match_attr[TAB_MATCH_MAX];

static void handle_tab() {
	if (input_size == 0)
		return;

	// find the start of the word being typed
	int tok_start = (int)input_size;
	while (tok_start > 0 && input_buffer[tok_start - 1] != ' ')
		tok_start--;
	int tok_len = (int)input_size - tok_start;
	bool completing_command = (tok_start == 0);

	int match_count = 0;

	if (completing_command) {
		for (int i = 0; commands[i].name != NULL && match_count < TAB_MATCH_MAX; i++) {
			if ((int)strlen(commands[i].name) >= tok_len && prefix_case(commands[i].name, input_buffer + tok_start)) {
				strcpy(tab_matches[match_count], commands[i].name);
				tab_match_attr[match_count] = 0;
				match_count++;
			}
		}
	} else {
		if (!is_hdd_present())
			return;
		nat32 total = fat32_file_count(current_dir_cluster) + fat32_dir_count(current_dir_cluster);
		for (nat32 i = 0; i < total && match_count < TAB_MATCH_MAX; i++) {
			fat32_entry_info_t info;
			if (!fat32_dir_get_entry(current_dir_cluster, i, &info))
				break;
			if ((int)strlen(info.name) >= tok_len && prefix_case(info.name, input_buffer + tok_start)) {
				strcpy(tab_matches[match_count], info.name);
				tab_match_attr[match_count] = info.attr;
				match_count++;
			}
		}
	}

	if (match_count == 0)
		return;

	if (match_count == 1) {
		// erase the partial token and print the completion
		for (int i = 0; i < tok_len; i++)
			do_backspace();
		input_size = (nat32)tok_start;

		const char* comp = tab_matches[0];
		for (int i = 0; comp[i] && input_size < sizeof(input_buffer) - 1; i++) {
			input_buffer[input_size++] = comp[i];
			echo_char(comp[i]);
		}

		// append suffix: space for commands '/' for directories
		char suffix = 0;
		if (completing_command)
			suffix = ' ';
		else if (tab_match_attr[0] & FAT32_ATTR_DIRECTORY)
			suffix = '/';
		if (suffix && input_size < sizeof(input_buffer) - 1) {
			input_buffer[input_size++] = suffix;
			echo_char(suffix);
		}
	} else {
		// multiple matches. list them then reprint the prompt + line
		do_clear_cursor();
		print("\n");
		for (int i = 0; i < match_count; i++) {
			print(tab_matches[i]);
			if (tab_match_attr[i] & FAT32_ATTR_DIRECTORY)
				print("/");
			print("  ");
			if ((i & 3) == 3)
				print("\n");
		}
		print("\n");
		printf("\\9x%s\\x$ ", current_dir);
		for (nat32 i = 0; i < input_size; i++)
			echo_char(input_buffer[i]);
	}
}

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
			if (!released && gfx_app_inited)
				translate_mesh(&cube, 0, 0, -move_amount);
			break;
		case 0x50: // down
			if (!released && gfx_app_inited)
				translate_mesh(&cube, 0, 0, move_amount);
			break;
		case 0x4B: // left
			if (!released && gfx_app_inited)
				translate_mesh(&cube, -move_amount, 0, 0);
			break;
		case 0x4D: // right
			if (!released && gfx_app_inited)
				translate_mesh(&cube, move_amount, 0, 0);
			break;
		}
		extended = false;
		return;
	}

	if (code > SC_MAX)
		return;

	if (code == LSHIFT || code == RSHIFT) {
		shift_pressed = !released;
		return;
	}

	if (released)
		return;
	if (gui_mode)
		return;

	switch (code) {
	case BACKSPACE:
		if (!released && gfx_app_inited) {
			translate_mesh(&cube, 0, 1.0f, 0);
		} else {
			if (input_size > 0) {
				input_size--;
				do_backspace();
			}
		}
		break;
	case ENTER:
		input_buffer[input_size] = '\0';
		do_clear_cursor();
		print("\n");
		ready = true;
		break;
	case TAB:
		handle_tab();
		break;
	default: {
		if (code == 0x39 && gfx_app_inited) {
			translate_mesh(&cube, 0, -1.0f, 0);
		} else {
			char letter = (shift_pressed || caps_lock_on) ? sc_ascii_shift[code] : sc_ascii[code];
			if (letter != '?' && input_size < (sizeof(input_buffer) - 1)) {
				input_buffer[input_size++] = letter;
				char str[2] = {letter, '\0'};
				print(str);
			}
		}
		break;
	}
	}
}

void init_keyboard() {
	register_interrupt_handler(33, keyboard_callback); // irq1
}

void sconf(char* buffer) {
	while (!ready)
		;
	strcpy(buffer, input_buffer);
	input_size = 0;
	ready = false;
}
