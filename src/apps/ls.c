#include "globals.h"
#include "apps/base.h"

void cmd_ls(const char** args, int argc) {
	nat32 total_files = fat32_file_count(current_dir_cluster);
	nat32 total_dirs  = fat32_dir_count(current_dir_cluster);
	nat32 total = total_files + total_dirs;

	if (!total) {
		print("no files or directories found\n");
		return;
	}

	for (nat32 i = 0; i < total; i++) {
		fat32_dir_entry_t entry;
		fat32_dir_get_entry(current_dir_cluster, i, &entry);

		if (entry.name[0] == 0 || entry.name[0] == 0xE5) continue;

		if (entry.attr & FAT32_ATTR_DIRECTORY) {
			printf("%s/\n", entry.name);
		} else {
			printf("%s    %d bytes\n", entry.name, entry.file_size);
		}
	}
}

void register_ls_cmd(void) {
    register_command(
		"ls",
		"lists files and directories in root",
		0,
		cmd_ls,
		0
	);
}
