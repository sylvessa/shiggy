#include "globals.h"
#include "apps/base.h"
#include "drivers/cmos.h"

void cmd_time(const char** args, int argc) {
	nat32 h = getRTCHours();
	nat32 m = getRTCMinutes();
	printf("%d:%d\n", h, m);
}

void register_time_cmd(void) {
    register_command(
		"time", // name
		"shows the time", // desc
		0, // hidden
		cmd_time, // func
		0 // args
	);
}