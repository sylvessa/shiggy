#include "globals.h"
#include "apps/base.h"

static void cmd_test(const char* args) {
	print("Normal text \\4FRed on white\\x back to white on black\n");
	print("\\2 Fuck\n");
	print("\\e1Yellow on blue\n");
}

static struct command_reg test_command = {
	.name = "test",
	.description = "tests whatever",
	.hidden = false,
	.func = cmd_test
};

__attribute__((used, section(".cmds"))) static struct command_reg* test_ptr = &test_command;
