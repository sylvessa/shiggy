#include "apps/base.h"
#include "globals.h"

void cmd_colors(const char** args, int argc) {
	print("\n");
	for (char color = 0; color <= 0x0f; color++) {
		for (int i = 0; i < 16; i++)
			print_char((unsigned char)219, color, color);
		print("\n");
	}
	print("\n");
}

command_t colors_cmd __attribute__((section(".cmds"))) = {
	.name = "colors",
	.description = "shows 16 colors",
	.hidden = 0,
	.func = cmd_colors,
	.args = 0};