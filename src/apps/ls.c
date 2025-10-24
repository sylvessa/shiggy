#include "globals.h"
#include "apps/base.h"

void cmd_ls(const char** args, int argc) {
	nat32 file_count = fat32_file_count();
	if (!file_count) {
		printf("no files found\n");
		return;
	}

	for (nat32 i = 0; i < file_count; i++) {
		const char* name = fat32_file_get_name(i);
		if (!name) continue;
		nat32 size = fat32_file_size(name);
		printf("%s    %d bytes\n", name, size);
	}
}

void register_ls_cmd(void) {
    register_command(
		"ls", // name
		"lists files (no args rn)", // desc
		0, // hidden
		cmd_ls, // func
		0 // args
	);
}