#include "apps/base.h"
#include "globals.h"
#include "network/main.h"

static char resp[4096];

static void cmd_nettest(const char** args, int argc) {
	(void)args; (void)argc;

	if (!net_initialized) init_network();

	int n = net_http_get("example.com", "/", resp, sizeof(resp));
	if (n < 0) return;

	printf("--- %d bytes ---\n", n);
	print(resp);
}

command_t nettest_cmd __attribute__((section(".cmds"))) = {
	.name = "nettest",
	.description = "http get example.com",
	.hidden = 0,
	.func = cmd_nettest,
	.args = 0};

