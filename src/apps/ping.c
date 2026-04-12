#include "apps/base.h"
#include "globals.h"
#include "network/main.h"

static int parse_ip(const char* s, byte out[4]) {
	for (int i = 0; i < 4; i++) {
		int n = 0;
		while (*s >= '0' && *s <= '9') n = n * 10 + (*s++ - '0');
		if (n > 255) return 0;
		out[i] = (byte)n;
		if (i < 3) { if (*s++ != '.') return 0; }
	}
	return 1;
}

static void cmd_ping(const char** args, int argc) {
	if (argc < 1) { print("usage: ping <ip|host>\n"); return; }

	if (!net_initialized) init_network();

	byte ip[4];
	if (!parse_ip(args[0], ip)) {
		printf("resolving %s ...\n", args[0]);
		if (!net_dns_resolve(args[0], ip)) {
			print("DNS failed\n");
			return;
		}
		printf("  -> %d.%d.%d.%d\n", ip[0], ip[1], ip[2], ip[3]);
	}

	printf("PING %d.%d.%d.%d\n", ip[0], ip[1], ip[2], ip[3]);

	int sent = 0, recv = 0;
	for (int i = 0; i < 4; i++) {
		nat32 rtt = 0;
		sent++;
		if (net_ping(ip, 5000, &rtt)) {
			recv++;
			printf("reply from %d.%d.%d.%d: seq=%d time=%d ms\n",
			       ip[0], ip[1], ip[2], ip[3], i + 1, rtt);
		} else {
			printf("request timeout for seq=%d\n", i + 1);
		}
	}

	printf("\n--- %d.%d.%d.%d ping statistics ---\n", ip[0], ip[1], ip[2], ip[3]);
	printf("%d packets transmitted, %d received, %d%% loss\n",
	       sent, recv, (sent - recv) * 100 / sent);
}

command_t ping_cmd __attribute__((section(".cmds"))) = {
	.name = "ping",
	.description = "ping an ip or hostname",
	.hidden = 0,
	.func = cmd_ping,
	.args = 1};
