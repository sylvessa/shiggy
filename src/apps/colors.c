#include "globals.h"
#include "apps/base.h"

void cmd_colors(const char** args, int argc) {
	print("\n"); 
    for (char color = 0; color <= 0x0f; color++) { 
        for (int i = 0; i < 16; i++) 
            print_char((unsigned char)219, color, color); 
        
        print("\n"); 
    } 
    print("\n");
}



__attribute__((used, section(".cmds"))) static struct command_reg colors_command = {
	.description = "shows 16 colors",
	.name = "colors",
	.hidden = false,
	.func = cmd_colors,
	.args = ARG_STRING
};
