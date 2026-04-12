#include "apps/base.h"
#include "globals.h"
#include "network/main.h"
#include "drivers/keyboard.h"

static int parse_ip(const char* s, byte out[4]) {
	for (int i = 0; i < 4; i++) {
		int n = 0;
		while (*s >= '0' && *s <= '9') n = n * 10 + (*s++ - '0');
		if (n > 255) return 0;
		out[i] = (byte)n;
		if (i < 3 && *s++ != '.') return 0;
	}
	return 1;
}

static nat16 parse_port(const char* s) {
	nat16 n = 0;
	while (*s >= '0' && *s <= '9') n = (nat16)(n * 10 + (*s++ - '0'));
	return n;
}

static int tcp_drain(tcp_conn_t* c, nat32 timeout_ms) {
	static byte buf[2048];
	nat32 start = timer_ticks, ticks = timeout_ms / 20;
	if (ticks == 0) ticks = 1;
	while (timer_ticks - start < ticks) {
		int n = net_tcp_recv(c, buf, sizeof(buf));
		if (n > 0) print((char*)buf);
		if (n < 0) return -1;
	}
	return 0;
}

static void cmd_ncat(const char** args, int argc) {
	int udp = 0, verbose = 0;
	const char* host_arg = NULL;
	const char* port_arg = NULL;

	for (int i = 0; i < argc; i++) {
		if (args[i][0] == '-') {
			const char* f = args[i] + 1;
			while (*f) {
				if (*f == 'u') udp = 1;
				else if (*f == 'v') verbose = 1;
				else { printf("ncat: unknown flag -%c\n", *f); return; }
				f++;
			}
		} else if (!host_arg) host_arg = args[i];
		else if (!port_arg) port_arg = args[i];
	}

	if (!host_arg || !port_arg) {
		print("usage: ncat [-u] [-v] <host> <port>\n");
		return;
	}

	if (!net_initialized) init_network();

	byte ip[4];
	if (!parse_ip(host_arg, ip)) {
		if (verbose) printf("resolving %s ...\n", host_arg);
		if (!net_dns_resolve(host_arg, ip)) { print("DNS failed\n"); return; }
		if (verbose) printf("  -> %d.%d.%d.%d\n", ip[0], ip[1], ip[2], ip[3]);
	}

	nat16 port = parse_port(port_arg);

	if (udp) {
		static byte recv_buf[1500];
		static char line[256];

		if (verbose) printf("udp -> %d.%d.%d.%d:%d\n", ip[0], ip[1], ip[2], ip[3], port);
		print("(udp mode, empty line to quit)\n");

		while (1) {
			sconf(line);
			if (!strlen(line)) break;

			net_udp_send_to(ip, port, (byte*)line, (nat16)strlen(line));
			if (verbose) printf("sent %d bytes\n", strlen(line));

			int n = net_udp_recv(54321, recv_buf, sizeof(recv_buf), 2000);
			if (n > 0) print((char*)recv_buf);
		}
		return;
	}

	// TCP
	static tcp_conn_t conn;
	static char line[256];
	static byte recv_buf[2048];

	if (verbose) printf("tcp -> %d.%d.%d.%d:%d ...\n", ip[0], ip[1], ip[2], ip[3], port);
	if (!net_tcp_open(&conn, ip, port)) { print("connect failed\n"); return; }
	if (verbose) print("connected\n");

	// catch server banner
	tcp_drain(&conn, 1000);

	while (!conn.peer_fin) {
		sconf(line);
		if (!strlen(line)) {
			net_tcp_close(&conn);
			break;
		}

		nat32 len = strlen(line);
		line[len] = '\r';
		line[len + 1] = '\n';
		line[len + 2] = '\0';
		net_tcp_send(&conn, (byte*)line, (nat16)(len + 2));
		if (verbose) printf("sent %d bytes\n", len + 2);

		nat32 start = timer_ticks, ticks = 3000 / 20;
		while (timer_ticks - start < ticks && !conn.peer_fin) {
			int n = net_tcp_recv(&conn, recv_buf, sizeof(recv_buf));
			if (n > 0) { print((char*)recv_buf); start = timer_ticks; }
			if (n < 0) goto done;
		}
	}

done:
	if (verbose) print("connection closed\n");
}

command_t ncat_cmd __attribute__((section(".cmds"))) = {
	.name = "ncat",
	.description = "tcp/udp client",
	.hidden = 0,
	.func = cmd_ncat,
	.args = 2};
