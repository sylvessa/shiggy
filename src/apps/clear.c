// apps/clear.c
#include "apps/base.h"
#include "globals.h"

void cmd_clear(const char** args, int argc) {
	screen_clear();
	print_center("=== shiggy - type help to get list of commands ===", 0x5);
}

command_t clear_cmd __attribute__((section(".cmds"))) = {
	.magic = COMMAND_MAGIC,
	.name = "clear",
	.description = "clears the terminal",
	.hidden = 0,
	.func = cmd_clear,
	.args = 0};