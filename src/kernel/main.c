#include "apps/base.h"
#include "cpu/acpi.h"
#include "cpu/timer.h"
#include "drivers/keyboard.h"
#include "drivers/mouse.h"
#include "drivers/vga.h"
#include "globals.h"

char* current_dir = "/";
int current_dir_cluster = FIRST_FILE_CLUSTER;

bool gui_mode = false;

void kmain() {
	char* input = malloc(255);

	isr_install();
	init_timer(50);
	init_ata();
	init_keyboard();

	print_center("=== shiggy - type help to get list of commands ===", 0x5);

	fat32_fs_init();

	if (acpi_init() != SUCCESS)
		print("Failed to initialized ACPI\n");

	register_all_commands();

	while (true) {
		if (!gui_mode) {
			printf("\\9x%s\\x$ ", current_dir);
		}

		sconf(input);

		bool handled = false;

		for (int i = 0; commands[i].name != NULL; i++) {
			strlower(input);
			int cmd_len = strlen(commands[i].name);
			if (strncmp(input, commands[i].name, cmd_len) == 0 && (input[cmd_len] == '\0' || input[cmd_len] == ' ')) {
				char* args_start = input + cmd_len;
				while (*args_start == ' ')
					args_start++;

				const char* argv[8];
				int argc = 0;

				if (*args_start != '\0') {
					char* token = args_start;
					while (*token && argc < 8) {
						argv[argc++] = token;
						while (*token && *token != ' ')
							token++;
						if (*token) {
							*token = '\0';
							token++;
						}
					}
				}

				commands[i].func(argv, argc);
				handled = true;
				break;
			}
		}

		if (!handled) {
			print(input);
			print(" is not a valid command.\n");
		}
	}
}
