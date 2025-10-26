#include "globals.h"
#include "apps/base.h"

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

void register_mk_cmd(void) {
	register_command(
		"mk",			// name
		"makes a file", // desc
		0,				// hidden
		cmd_mk,			// func
		1				// args
	);
}