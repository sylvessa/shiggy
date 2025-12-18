#include "apps/base.h"
#include "globals.h"

static void cmd_diskinfo(const char** args, int argc) {
	if (is_hdd_present()) {
		print("Found IDE Drive\n");
		printf("%d MB\n", ata_get_drive_size() / 2048);

		int formatted = is_formatted();
		printf("formatted: %s\n", formatted ? "yes" : "no");
	} else {
		print("no hdd\n");
	}
}

command_t diskinfo_cmd __attribute__((section(".cmds"))) = {
	.name = "diskinfo",
	.description = "displays hdd drive info",
	.hidden = 0,
	.func = cmd_diskinfo,
	.args = 0};