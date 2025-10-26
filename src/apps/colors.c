#include "globals.h"
#include "apps/base.h"

void cmd_colors(const char** args, int argc) {
	print("\n");
	for (char color = 0; color <= 0x0f; color++) {
		for (int i = 0; i < 16; i++)
			print_char((unsigned char)219, color, color);
		print("\n");
	}
	print("\n");
}

void register_colors_cmd(void) {
	register_command(
		"colors",		   // name
		"shows 16 colors", // desc
		0,				   // hidden
		cmd_colors,		   // func
		0				   // args
	);
}