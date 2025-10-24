#include "globals.h"
#include "apps/base.h"
#include "graphics/main.h"
#include "lib/math.h"

typedef struct { int x, y; } point;

point base_square[4];
point square[4];
point center;
float angle = 0;
float speed = 0.01f;
int inited = 0;

void update_square() {
	for(int i = 0; i < 4; i++) {
		int dx = base_square[i].x - center.x;
		int dy = base_square[i].y - center.y;
		int nx = (int)(dx * cos(angle) - dy * sin(angle));
		int ny = (int)(dx * sin(angle) + dy * cos(angle));
		square[i].x = center.x + nx;
		square[i].y = center.y + ny;
	}
}

void draw_square(int color) {
	gfx_draw_line(square[0].x, square[0].y, square[1].x, square[1].y, color, 3);
	gfx_draw_line(square[1].x, square[1].y, square[2].x, square[2].y, color, 3);
	gfx_draw_line(square[2].x, square[2].y, square[3].x, square[3].y, color, 3);
	gfx_draw_line(square[3].x, square[3].y, square[0].x, square[0].y, color, 3);
}

void timer_callback() {
	if(!inited) return;
	draw_square(0xA);
	angle += speed;
	if(angle > 6.283185f) angle -= 6.283185f;
	update_square();
	draw_square(0xF);
}

void cmd_gfx(const char** args, int argc) {
	if(inited) {
		inited = 0;
		draw_square(0x0);
		return;
	}

	if(argc >= 1) {
		speed = atof(args[0]);
		if (speed > 1) {
			print("too much speeeed\n");
			return;
		}
	}

	base_square[0] = (point){200, 200};
	base_square[1] = (point){300, 200};
	base_square[2] = (point){300, 300};
	base_square[3] = (point){200, 300};

	center = (point){250, 250};

	angle = 0;
	update_square();
	draw_square(0xF);

	register_interrupt_handler(32, timer_callback);
	inited = 1;
}

void register_gfx_cmd(void) {
	register_command(
		"gfx", 
		"shows rotating square. optional arg for rotate speed (gfx <speed>)", 
		0, 
		cmd_gfx, 
		1
	);
}
