#include "apps/base.h"
#include "globals.h"

void cmd_cd(const char** args, int argc) {
	if (argc < 1) {
		print("usage: cd <directory>\n");
		return;
	}

	char* target = (char*)args[0];

	if (strcmp(target, "/") == 0) {
		current_dir_cluster = FIRST_FILE_CLUSTER;
		strcpy(current_dir, "/");
		return;
	}

	if (strcmp(target, "..") == 0) {
		if (current_dir_cluster == FIRST_FILE_CLUSTER)
			return; // already root llol

		for (int i = 0; i < MAX_ROOT_ENTRIES; i++) {
			if (root_dir[i].name[0] == 0)
				continue;
			if (root_dir[i].first_cluster == current_dir_cluster) {
				current_dir_cluster = root_dir[i].parent_cluster;

				// remove last component from current_dir string
				char* slash = strrchr(current_dir, '/');
				if (slash != NULL && slash != current_dir)
					*slash = 0;
				else
					strcpy(current_dir, "/");
				break;
			}
		}
		return;
	}

	nat32 total_dirs = fat32_dir_count(current_dir_cluster);
	fat32_dir_entry_t entry;
	nat8 found = 0;

	for (nat32 i = 0; i < total_dirs; i++) {
		fat32_dir_get_entry(current_dir_cluster, i, &entry);
		if ((entry.attr & FAT32_ATTR_DIRECTORY) && strncmp((char*)entry.name, target, strlen(target)) == 0) {
			current_dir_cluster = entry.first_cluster;

			if (strcmp(current_dir, "/") != 0)
				strcat(current_dir, "/");
			strcat(current_dir, target);
			found = 1;
			break;
		}
	}

	if (!found)
		print("directory not found\n");
	// else printf("went to %d cluster\n", current_dir_cluster);
}

void register_cd_cmd(void) {
	register_command(
		"cd",				   // name
		"go into a directory", // desc
		0,					   // hidden
		cmd_cd,				   // func
		1					   // args
	);
}

command_t cd_cmd __attribute__((section(".cmds"))) = {
	.magic = COMMAND_MAGIC,
	.name = "cd",
	.description = "go into a directory",
	.hidden = 0,
	.func = cmd_cd,
	.args = 1};