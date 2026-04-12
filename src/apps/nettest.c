#include "apps/base.h"
#include "globals.h"
#include "network/main.h"

static byte google_ip[4] = {8, 8, 8, 8};

static int net_ready = 0;

static void cmd_nettest(const char** args, int argc) {
	(void)args; (void)argc;

	// if (!net_ready) {
	// 	init_network();
	// 	net_ready = 1;
	// }

	print("PING 8.8.8.8 (google.com) 32 bytes of data\n");

	int sent = 0, recv = 0;
	for (int i = 0; i < 4; i++) {
		nat32 rtt = 0;
		sent++;
		int ok = net_ping(google_ip, 5000, &rtt);
		if (ok) {
			recv++;
			printf("reply from 8.8.8.8: seq=%d time=%d ms\n", i + 1, rtt);
		} else {
			printf("request timeout for seq=%d\n", i + 1);
		}
	}

	printf("\n--- 8.8.8.8 ping statistics ---\n");
	printf("%d packets sent, %d received, %d%% loss\n",
	       sent, recv, (sent - recv) * 100 / sent);
}

command_t nettest_cmd __attribute__((section(".cmds"))) = {
	.name = "nettest",
	.description = "ping 8.8.8.8 (google)",
	.hidden = 0,
	.func = cmd_nettest,
	.args = 0};
