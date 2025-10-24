#include "globals.h"
#include "drivers/keyboard.h"
#include "cpu/timer.h"
#include "apps/base.h"
#include "cpu/acpi.h"
#include "drivers/vga.h"

void kmain() {
	unsigned long kernel_start = (unsigned long)&kernel_start_addr;
    unsigned long heap_start = (unsigned long)&__free_memory_start;
	
	char input[255];

	isr_install();
	init_timer(50);
	init_ata();
	init_vga_text();
	init_keyboard();
	
	print_center("=== shiggy - type help to get list of commands ===", 0x5);

	fat32_fs_init();

	if (acpi_init() != SUCCESS)
		print("Failed to initialized ACPI\n");


	//register_all_commands_from_section();
	register_all_commands();

	while (true) {
		print("\\2xOS >\\x ");
		sconf(input);

		bool handled = false;

		for (int i = 0; commands[i].name != NULL; i++) {
			strlower(input);

			int cmd_len = strlen((char*)commands[i].name);

			if (strncmp(input, (char*)commands[i].name, cmd_len) == 0) {
				char *args_start = input + cmd_len;
				while (*args_start == ' ')
					args_start++;

				const char* argv[8];
				int argc = 0;

				if (*args_start != '\0') {
					char* token = args_start;
					while (*token && argc < 8) {
						argv[argc++] = token;

						while (*token && *token != ' ') token++;
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
