#include "apps/base.h"
#include "globals.h"

void cmd_mk(const char** args, int argc) {
	if (argc < 1) {
		print("usage: mk <filename>\n");
		return;
	}

	char* filename = (char*)args[0];
	char content[128];
	snprintf(content, sizeof(content), "this is da content for %s", filename);
	fat32_create_file(current_dir_cluster, filename, content);
	printf("created file %s\n", filename);
	free(content);
}

command_t mk_cmd __attribute__((section(".cmds"))) = {
	.magic = COMMAND_MAGIC,
	.name = "mk",
	.description = "makes a file",
	.hidden = 0,
	.func = cmd_mk,
	.args = 1};