#include "globals.h"
#include "apps/base.h"
#include "lib/string.h"
#include "drivers/ata.h"

#include "fs/fat32_file.h"

static void cmd_mk(const char** args, int argc) {
	fat32_file_make("TEST.TXT");
	fat32_file_write_str("TEST.TXT", "sFuck");
	print("mdae file\n");
}

static struct command_reg mk_command = {
	.name = "mk",
	.description = "test command to make a file",
	.hidden = false,
	.func = cmd_mk,
};

__attribute__((used, section(".cmds"))) static struct command_reg* mk_ptr = &mk_command;
