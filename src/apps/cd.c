#include "apps/base.h"
#include "globals.h"

void cmd_cd(const char** args, int argc) {
	if (argc < 1) {
		print("usage: cd <directory>\n");
		return;
	}

	if (!is_hdd_present()) {
		print("No HDD detected!\n");
		return;
	}

	char* target = (char*)args[0];

	if (strcmp(target, "/") == 0) {
		current_dir_cluster = FAT32_ROOT_CLUSTER;
		strcpy(current_dir, "/");
		return;
	}

	if (strcmp(target, "..") == 0) {
		if (current_dir_cluster == FAT32_ROOT_CLUSTER)
			return; // already root lol

		nat32 parent;
		if (fat32_dir_parent(current_dir_cluster, &parent)) {
			current_dir_cluster = parent;

			// remove last component from current_dir string
			char* slash = strrchr(current_dir, '/');
			if (slash != NULL && slash != current_dir)
				*slash = 0;
			else
				strcpy(current_dir, "/");
		}
		return;
	}

	nat32 total = fat32_file_count(current_dir_cluster) + fat32_dir_count(current_dir_cluster);

	for (nat32 i = 0; i < total; i++) {
		fat32_entry_info_t info;
		if (!fat32_dir_get_entry(current_dir_cluster, i, &info))
			break;

		if ((info.attr & FAT32_ATTR_DIRECTORY) && strcasecmp(info.name, target) == 0) {
			current_dir_cluster = info.first_cluster;

			if (strcmp(current_dir, "/") != 0)
				strcat(current_dir, "/");
			strcat(current_dir, target);
			return;
		}
	}

	print("directory not found\n");
}

command_t cd_cmd __attribute__((section(".cmds"))) = {
	.name = "cd",
	.description = "go into a directory",
	.hidden = 0,
	.func = cmd_cd,
	.args = 1};