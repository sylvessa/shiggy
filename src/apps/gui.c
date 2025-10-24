#include "globals.h"
#include "apps/base.h"

void cmd_gui(const char** args, int argc) {
	gui_mode = true;

	screen_clear();

	print_center("soon", 0x00);
}

void register_gui_cmd(void) {
    register_command(
		"gui", // name
		"gui mode", // desc
		0, // hidden
		cmd_gui, // func
		0 // args
	);
}