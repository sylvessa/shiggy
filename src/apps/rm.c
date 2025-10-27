#include "apps/base.h"
#include "globals.h"

void cmd_rm(const char** args, int argc) {
	if (argc < 1) {
		print("usage: rm <filename>\n");
		return;
	}

	char* filename = (char*)args[0];

	if (!fat32_delete_file(current_dir_cluster, filename)) {
		print("file not found or could not be deleted\n");
		return;
	}

	print("file deleted\n");
}

command_t rm_cmd __attribute__((section(".cmds"))) = {
	.name = "rm",
	.description = "removes a file",
	.hidden = 0,
	.func = cmd_rm,
	.args = 1};
