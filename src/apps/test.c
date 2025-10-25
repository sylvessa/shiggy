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

	void *phys_addr = malloc(n);
	printf("free mem starts at %p\nallocated %d bytes at address %p\n", &__free_memory_start, n, phys_addr);
	sleep_s(4);
}

void register_test_cmd(void) {
    register_command(
		"test", // name
		"test command", // desc
		0, // hidden
		cmd_test, // func
		1 // args
	);
}