#include "globals.h"
#include "apps/base.h"

void cmd_colors(const char* args) {
	print("\n"); 
    for (char color = 0; color <= 0x0f; color++) { 
        for (int i = 0; i < 16; i++) 
            print_char(219, -1, -1, color); 
        
        print("\n"); 
    } 
    print("\n");
}
