#include "globals.h"
#include "apps/base.h"
#include "fs/fat32.h"

void cmd_mk(const char** args, int argc) {
	if (argc < 1) {
		print("usage: mk <filename>\n");
		return;
	}

	char* filename = (char*)args[0];
	char content[128];
	snprintf(content, sizeof(content), "this is da content for %s", filename);
	fat32_create_file(filename, content);
}

static struct command_reg mk_command = {
	.name = "mk",
	.description = "makes a file",
	.hidden = false,
	.func = cmd_mk,
};

__attribute__((used, section(".cmds"))) static struct command_reg* mk_ptr = &mk_command;
