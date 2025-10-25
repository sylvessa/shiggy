#include "globals.h"
#include "apps/base.h"

void draw_smiley(int start_x, int start_y) {
	nat8 fg = 15;
	nat8 bg = 0;
	int scale = 10; // 16x16 -> 80x80

	nat8 smiley[16][16] = {
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,1,1,0,0,0,0,0,0,1,1,0,0,0},
		{0,0,0,1,1,0,0,0,0,0,0,1,1,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,1,0,0,0,0,0,0,0,0,1,0,0,0},
		{0,0,0,1,0,0,0,0,0,0,0,0,1,0,0,0},
		{0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0},
		{0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
	};


	for(int y=0; y<16; y++) {
		for(int x=0; x<16; x++) {
			nat8 color = smiley[y][x] ? fg : bg;

			// draw 5x5 block for scaling
			for(int dy=0; dy<scale; dy++) {
				for(int dx=0; dx<scale; dx++) {
					draw_pixel(start_x + x*scale + dx, start_y + y*scale + dy, color);
				}
			}
		}
	}
}

void cmd_gui(const char** args, int argc) {
	gui_mode = true;

	screen_clear();
	//set_bg_color(0x0F);

	draw_smiley(400, 50);
	print_center("soon", 0x00);
}

void register_gui_cmd(void) {
    register_command(
		"gui", // name
		"gui mode", // desc
		0, // hidden
		cmd_gui, // func
		0 // args
	);
}