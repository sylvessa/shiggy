#include "globals.h"
#include "apps/base.h"
#include "fs/fat32_file.h"

void cmd_ls(const char** args, int argc) {
    for (int i = 0; i < fat32_file_count(); i++) {
        const char* name = fat32_file_get_name(i);
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
