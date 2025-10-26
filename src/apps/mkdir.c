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

void register_mkdir_cmd(void) {
	register_command(
		"mkdir",		// name
		"makes a file", // desc
		0,				// hidden
		cmd_mkdir,		// func
		1				// args
	);
}