#include "globals.h"
#include "apps/base.h"

void cmd_ls(const char* args) {
    (void)args;
    for (int i = 0; i < file_count(); ++i) {
        char* name = file_get_name(i);
        if (name != NULL)
            printf("%s\n", name);
    }
}


static struct command_reg ls_command = {
	.name = "ls",
	.description = "lists files (no args rn)",
	.hidden = false,
	.func = cmd_ls
};

__attribute__((used, section(".cmds"))) static struct command_reg* ls_ptr = &ls_command;
