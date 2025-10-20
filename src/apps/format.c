#include "globals.h"
#include "apps/base.h"
#include "fs/fat32.h"

void cmd_format(const char** args, int argc) {
	fat32_format();
}

static struct command_reg format_command = {
	.name = "format",
	.description = "formats HDD if one is present",
	.hidden = false,
	.func = cmd_format
};

__attribute__((used, section(".cmds"))) static struct command_reg* format_ptr = &format_command;
