#include "globals.h"
#include "drivers/screen.h"
#include "drivers/keyboard.h"
#include "cpu/timer.h"
#include "cpu/acpi.h"

void kmain() {
    char input[255];

	screen_clear();
	print("funny os thing\n");

	isr_install();
	init_timer(50);
	
    init_keyboard();

	if (acpi_init() != SUCCESS)
        print("Failed to initialized ACPI\n");

	print("type seoemthing keyboard inited :3\n\n");

	while (true) {
		print("\\2OS >\\f ");
		sconf(input);

		if (strcmp(input, "help") == 0) {
			print("made by Me\n\n");
		} else {
			print(input);
			print("\n");
		}
	}
}
