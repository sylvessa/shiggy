#include "globals.h"
#include "apps/base.h"

void cmd_clear(const char** args, int argc) {
	screen_clear();
	print_centered("=== shiggy - type help to get list of commands ===", 0x5);
}

static struct command_reg clear_command = {
	.name = "clear",
	.description = "clears the terminal",
	.hidden = false,
	.func = cmd_clear
};

__attribute__((used, section(".cmds"))) static struct command_reg* clear_ptr = &clear_command;
