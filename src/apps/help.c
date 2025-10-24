#include "globals.h"
#include "apps/base.h"

static void cmd_help(const char** args, int argc) {
	print("this is an experimental os written by \\5xsylvessa\\x\n\n");
	print("written in C and a bit of x86 assembly\n");
	print("bootloader and everything was made by me.\n");
	print("\ncommands:\n"); 
	for (int i = 0; i < command_count; i++) { 
		if (!commands[i].hidden) { 
			print((char*)commands[i].name); 
			print(" - "); 
			print((char*)commands[i].description); 
			print("\n"); 
		} 
	} 
}

__attribute__((used, section(".cmds"))) static struct command_reg help_command = {
	.name = "help",
	.description = "shows this help message",
	.hidden = false,
	.func = cmd_help
};
