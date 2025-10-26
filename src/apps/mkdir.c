#include "apps/base.h"
#include "globals.h"

void cmd_mkdir(const char** args, int argc) {
	if (argc < 1) {
		print("usage: mkdir <name>\n");
		return;
	}

	char* dirname = (char*)args[0];
	if (fat32_create_dir(current_dir_cluster, dirname)) {
		printf("created directory %s/\n", dirname);
	} else {
		print("failed to create directory\n");
	}
}

command_t mkdir_cmd __attribute__((section(".cmds"))) = {
	.name = "mkdir",
	.description = "makes a directory",
	.hidden = 0,
	.func = cmd_mkdir,
	.args = 1};