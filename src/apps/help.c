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

void register_help_cmd(void) {
    register_command(
		"help", // name
		"shows this help message", // desc
		0, // hidden
		cmd_help, // func
		0 // args
	);
}