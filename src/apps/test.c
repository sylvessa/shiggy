#include "globals.h"
#include "apps/base.h"
#include "lib/string.h"
#include "drivers/ata.h"
#include "fs/fat32.h"

static void cmd_test(const char** args, int argc) {
	// if (argc < 1) {
	// 	print("usage: test <num_bytes>\n");
	// 	return;
	// }

	// int n = 0;
	// for (int i = 0; args[0][i] != '\0'; i++) {
	// 	if (args[0][i] < '0' || args[0][i] > '9') {
	// 		print("argument must be a positive number\n");
	// 		return;
	// 	}
	// 	n = n * 10 + (args[0][i] - '0');
	// }

	// int *phys_addr = (int*)malloc(n);
	// printf("allocated %d bytes at address %p\n", n, phys_addr);
	ata_identify();
	int formatted = fat32_is_formatted();
	printf("formatted: %s\n", formatted ? "yes" : "no");
}

static struct command_reg test_command = {
	.name = "test",
	.description = "test command",
	.hidden = false,
	.func = cmd_test,
	.args = ARG_NUMBER
};

__attribute__((used, section(".cmds"))) static struct command_reg* test_ptr = &test_command;
