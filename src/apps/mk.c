#include "apps/base.h"
#include "globals.h"

void cmd_mk(const char** args, int argc) {
	if (argc < 2) {
		print("usage: mk <filename> <content...>\n");
		return;
	}

	char* filename = (char*)args[0];

	int content_len = 0;
	for (int i = 1; i < argc; i++)
		content_len += strlen(args[i]) + 1;

	char* content = (char*)malloc(content_len);
	if (!content) return;

	content[0] = 0;
	for (int i = 1; i < argc; i++) {
		strcat(content, args[i]);
		if (i != argc - 1) strcat(content, " ");
	}

	if (fat32_create_file(current_dir_cluster, filename, content))
		printf("created file %s\n", filename);
	else
		print("failed to create file\n");

	free(content);
}

command_t mk_cmd __attribute__((section(".cmds"))) = {
	.name = "mk",
	.description = "makes a file with content",
	.hidden = 0,
	.func = cmd_mk,
	.args = 2};
