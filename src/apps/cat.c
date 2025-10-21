#include "globals.h"
#include "apps/base.h"

void cmd_cat(const char** args, int argc) {
    if (argc < 1) {
		print("usage: cat <filename>\n");
		return;
	}

    // char* filename = (char*)args[0];

    // char* content = (char*) malloc(file_size(filename));
    // int response = file_read(filename, content);
    // if (response == FILE_NOT_FOUND)
    //     print("File not found\n");
    // else {
    //     print(content);
    //     print("\n");
    // }
    
    // free(content);
}


static struct command_reg cat_command = {
	.name = "cat",
	.description = "shows contents of a file",
	.hidden = false,
	.func = cmd_cat,
	.args = ARG_STRING
};

__attribute__((used, section(".cmds"))) static struct command_reg* cat_ptr = &cat_command;
