#include "globals.h"
#include "apps/base.h"

static nat8 rand_color() {
	return (rand() % 15) + 1; // 1-15, avoiding black
}

static void cmd_help(const char** args, int argc) {
	print("this is an experimental os written by \\5xsylvessa\\x\n\n");
	print("written in C and a bit of x86 assembly\n");
	print("bootloader and everything was made by me.\n");
	print("\ncommands:\n"); 

	for (int i = 0; i < command_count; i++) { 
		if (!commands[i].hidden) { 
			nat8 c = rand_color();
			for (const char* s = commands[i].name; *s; s++) {
				print_char(*s, c, 0x00);
			}
			print(" - "); 
			print((char*)commands[i].description); 
			print("\n"); 
		} 
	}

	print("\n");
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