#include "globals.h"
#include "drivers/screen.h"
#include "drivers/keyboard.h"
#include "cpu/timer.h"
#include "apps/base.h"
#include "cpu/acpi.h"

void kmain() {
    char input[255];

	screen_clear();
	print_centered("=== shiggy - type help to get list of commands ===", 0x5);
	isr_install();
	init_timer(50);
	
    init_keyboard();

	if (acpi_init() != SUCCESS)
        print("Failed to initialized ACPI\n");
	
	register_all_commands_from_section();

	while (true) {
		print("\\2xOS >\\x ");
		sconf(input);

		bool handled = false;

		for (int i = 0; commands[i].name != NULL; i++) {
			strlower(input);
			if (strcmp(input, (char*)commands[i].name) == 0) {
				commands[i].func(NULL);
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
