#include "globals.h"
#include "apps/base.h"

void cmd_colors(const char* args) {
    (void)args;
	print("\n"); 
    for (char color = 0; color <= 0x0f; color++) { 
        for (int i = 0; i < 16; i++) 
            print_char((unsigned char)219, -1, -1, color); 
        
        print("\n"); 
    } 
    print("\n");
}

static struct command_reg colors_command = {
	.name = "colors",
	.description = "shows 16 colors",
	.hidden = false,
	.func = cmd_colors
};

__attribute__((used, section(".cmds"))) static struct command_reg* colors_ptr = &colors_command;
