#include "apps/base.h"
#include "globals.h"

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

	void* phys_addr = malloc(n);
	printf("free mem starts at %p\nallocated %d bytes at address %p\n", &__free_memory_start, n, phys_addr);
}

command_t test_cmd __attribute__((section(".cmds"))) = {
	.magic = COMMAND_MAGIC,
	.name = "test",
	.description = "test command",
	.hidden = 1,
	.func = cmd_test,
	.args = 1};