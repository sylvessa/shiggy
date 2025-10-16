#include "globals.h"
#include "apps/base.h"

static void cmd_help(const char* args) {
	(void)args;
	print("this is an experimental os written by \\5xsylvessa\\x\n\n");
	print("written in C and a bit of x86 assembly\n");
	print("theres no file system or anything (yet), this is entirely just kernel.\n\n");
	print("bootloader and everything was made by me.\n");
	list_commands();
}

static struct command_reg help_command = {
	.name = "help",
	.description = "shows this help message",
	.hidden = false,
	.func = cmd_help
};

__attribute__((used, section(".cmds"))) static struct command_reg* help_ptr = &help_command;
