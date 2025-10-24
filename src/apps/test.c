#include "globals.h"
#include "apps/base.h"

static void cmd_test(const char** args, int argc) {
	if (argc < 1) {
		print("usage: test <num_bytes>\n");
		return;
	}

	int n = 0;
	for (int i = 0; args[0][i] != '\0'; i++) {
		if (args[0][i] < '0' || args[0][i] > '9') {
			print("argument must be a positive number\n");
			return;
		}
		n = n * 10 + (args[0][i] - '0');
	}

	int *phys_addr = (int*)malloc(n);
	printf("allocated %d bytes at address %p\n", n, phys_addr);
}

void register_test_cmd(void) {
    register_command(
		"test", // name
		"test command", // desc
		1, // hidden
		cmd_test, // func
		1 // args
	);
}