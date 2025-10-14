#include "globals.h"
#include "drivers/screen.h"
#include "drivers/keyboard.h"
#include "cpu/timer.h"
#include "apps/base.h"
#include "cpu/acpi.h"
#include "lib/ext2.h"

void kmain() {
    char input[255];

	screen_clear();
	print("funny os thing\n");

	// Read filesystem info from physical 0x0800 (bootloader writes it there)
	nat32 fs_addr = *(nat16*)0x0800 | ((nat32)*(nat16*)0x0802 << 16);
	nat16 fs_sectors = *(nat16*)0x0804;
	ext2_init(fs_addr, fs_sectors);

	isr_install();
	init_timer(50);
	
    init_keyboard();

	if (acpi_init() != SUCCESS)
        print("Failed to initialized ACPI\n");

	print("type seoemthing keyboard inited :3\n\n");

	while (true) {
		print("\\2OS >\\f ");
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
