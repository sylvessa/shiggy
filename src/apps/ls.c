#include "apps/base.h"
#include "globals.h"

void cmd_ls(const char** args, int argc) {
	if (!is_hdd_present()) {
		print("No HDD detected!\n");
		return;
	}

	nat32 total = fat32_file_count(current_dir_cluster) + fat32_dir_count(current_dir_cluster);

	if (!total) {
		print("no files or directories found\n");
		return;
	}

	for (nat32 i = 0; i < total; i++) {
		fat32_entry_info_t info;
		if (!fat32_dir_get_entry(current_dir_cluster, i, &info))
			break;

		if (info.attr & FAT32_ATTR_DIRECTORY)
			printf("%s/\n", info.name);
		else
			printf("%s    %d bytes\n", info.name, info.file_size);
	}
}

command_t ls_cmd __attribute__((section(".cmds"))) = {
	.name = "ls",
	.description = "lists files and directories in current dir",
	.hidden = 0,
	.func = cmd_ls,
	.args = 0};