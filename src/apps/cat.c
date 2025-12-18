#include "apps/base.h"
#include "globals.h"

void cmd_cat(const char** args, int argc) {
	if (argc < 1) {
		print("usage: cat <filename>\n");
		return;
	}

	if (!is_hdd_present()) {
		print("No HDD detected!\n");
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

command_t cat_cmd __attribute__((section(".cmds"))) = {
	.name = "cat",
	.description = "shows contents of a file",
	.hidden = 0,
	.func = cmd_cat,
	.args = 1};