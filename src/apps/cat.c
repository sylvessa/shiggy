#include "globals.h"
#include "apps/base.h"

void cmd_cat(const char** args, int argc) {
	if (argc < 1) {
		print("usage: cat <filename>\n");
		return;
	}

	char* filename = (char*)args[0];

	nat32 sz = fat32_file_size(filename);
	if (sz == 0) {
		print("File not found\n");
		return;
	}

	char* content = (char*)malloc(sz + 1);
	if (!content) {
		print("Memory allocation failed\n");
		return;
	}

	if (!fat32_read_file(filename, content, sz + 1)) {
		print("Failed to read file\n");
		free(content);
		return;
	}

	print(content);
	print("\n");
	free(content);
}



__attribute__((used, section(".cmds"))) static struct command_reg cat_command = {
	.name = "cat",
	.description = "shows contents of a file",
	.hidden = false,
	.func = cmd_cat,
	.args = ARG_STRING
};
