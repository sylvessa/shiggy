#include "apps/base.h"
#include "drivers/cmos.h"
#include "globals.h"

void cmd_time(const char** args, int argc) {
	nat32 h = getRTCHours();
	nat32 m = getRTCMinutes();
	printf("%d:%d\n", h, m);
}

command_t time_cmd __attribute__((section(".cmds"))) = {
	.name = "time",
	.description = "shows the time",
	.hidden = 0,
	.func = cmd_time,
	.args = 0};