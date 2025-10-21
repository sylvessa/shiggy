#include "globals.h"
#include "apps/base.h"
#include "lib/string.h"
#include "drivers/ata.h"

static void cmd_diskinfo(const char** args, int argc) {
	if (ata_identify()) {
		print("Found IDE Drive\n");
		printf("%d MB\n", ata_get_drive_size() / 2048);

		//int formatted = fat32_is_formatted();
		//printf("formatted: %s\n", formatted ? "yes" : "no");
	} else {
		print("no hdd\n");
	}
}

static struct command_reg diskinfo_command = {
	.name = "diskinfo",
	.description = "displays hdd drive info",
	.hidden = false,
	.func = cmd_diskinfo
};

__attribute__((used, section(".cmds"))) static struct command_reg* diskinfo_ptr = &diskinfo_command;
