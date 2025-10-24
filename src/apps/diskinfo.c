#include "globals.h"
#include "apps/base.h"

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

void register_diskinfo_cmd(void) {
    register_command(
		"diskinfo", // name
		"displays hdd drive info", // desc
		0, // hidden
		cmd_diskinfo, // func
		0 // args
	);
}