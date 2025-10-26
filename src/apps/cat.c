#include "globals.h"
#include "apps/base.h"

void cmd_cat(const char** args, int argc) {
	if (argc < 1) {
		print("usage: cat <filename>\n");
		return;
	}

	char* filename = (char*)args[0];

	nat32 sz = fat32_file_size(current_dir_cluster, filename);
	if (sz == 0) {
		print("File not found\n");
		return;
	}

	char* content = (char*)malloc(sz + 1);
	if (!content) {
		return;
	}

	if (!fat32_read_file(current_dir_cluster, filename, content, sz + 1)) {
		print("Failed to read file\n");
		free(content);
		return;
	}

	print(content);
	print("\n");
	free(content);
}

void register_cat_cmd(void) {
	register_command(
		"cat",						// name
		"shows contents of a file", // desc
		0,							// hidden
		cmd_cat,					// func
		1							// args
	);
}