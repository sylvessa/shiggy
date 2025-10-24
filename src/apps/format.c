#include "globals.h"
#include "apps/base.h"

void cmd_format(const char** args, int argc) {
	//fat32_format();
	print("drive formatted (not really get pranked)\n");
}

void register_format_cmd(void) {
    register_command(
		"format", // name
		"formats HDD if one is present", // desc
		0, // hidden
		cmd_format, // func
		0 // args
	);
}