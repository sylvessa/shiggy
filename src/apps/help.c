#include "globals.h"
#include "apps/base.h"

static void cmd_help(const char* args) {
	(void)args; // silence unused parameter warning
	print("this is an experimental os written by \\5xsylvessa\\x\n");
	print("lol\n");
}

static struct command_reg help_command = {
	.name = "help",
	.description = "Shows this help message",
	.hidden = false,
	.func = cmd_help
};

__attribute__((used, section(".cmds"))) static struct command_reg* help_ptr = &help_command;
