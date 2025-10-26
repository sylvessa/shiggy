#include "apps/base.h"
#include "globals.h"
#include "network/main.h"

static void cmd_nettest(const char** args, int argc) {
	init_network();
}

command_t nettest_cmd __attribute__((section(".cmds"))) = {
	.name = "nettest",
	.description = "network test",
	.hidden = 1,
	.func = cmd_nettest,
	.args = 0};