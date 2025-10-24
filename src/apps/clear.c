#include "globals.h"
#include "apps/base.h"

void cmd_clear(const char** args, int argc) {
	screen_clear();
	print_center("=== shiggy - type help to get list of commands ===", 0x5);
}

void register_clear_cmd(void) {
    register_command(
		"clear", // name
		"shows 16 clear", // desc
		0, // hidden
		cmd_clear, // func
		0 // args
	);
}