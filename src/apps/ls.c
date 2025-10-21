#include "globals.h"
#include "apps/base.h"
#include "fs/fat32.h"

void cmd_ls(const char** args, int argc) {
	nat32 file_count = fat32_file_count();
	if (!file_count) {
		printf("no files found\n");
		return;
	}

	for (nat32 i = 0; i < file_count; i++) {
		const char* name = fat32_file_get_name(i);
		if (!name) continue;
		nat32 size = fat32_file_size(name);
		printf("%s    %d bytes\n", name, size);
	}
}

static struct command_reg ls_command = {
    .name = "ls",
    .description = "lists files (no args rn)",
    .hidden = false,
    .func = cmd_ls
};

__attribute__((used, section(".cmds"))) static struct command_reg* ls_ptr = &ls_command;
