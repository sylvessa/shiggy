#include "globals.h"
#include "apps/base.h"
#include "graphics/main.h"
#include "lib/math.h"

typedef struct { int x, y; } point;

point triangle[3];
point center;
int inited = 0;

static unsigned int rng_seed = 12345;

unsigned int rand_int() {
	rng_seed = rng_seed * 1103515245 + 12345;
	return (rng_seed / 65536) % 32768;
}

float rand_float() {
	return rand_int() / 32768.0f;
}

void draw_triangle(int color) {
	gfx_draw_line(triangle[0].x, triangle[0].y, triangle[1].x, triangle[1].y, color, 3);
	gfx_draw_line(triangle[1].x, triangle[1].y, triangle[2].x, triangle[2].y, color, 3);
	gfx_draw_line(triangle[2].x, triangle[2].y, triangle[0].x, triangle[0].y, color, 3);
}

void rotate_triangle(float angle) {
	for(int i = 0; i < 3; i++) {
		int dx = triangle[i].x - center.x;
		int dy = triangle[i].y - center.y;

		int nx = (int)(dx * cos(angle) - dy * sin(angle));
		int ny = (int)(dx * sin(angle) + dy * cos(angle));

		triangle[i].x = center.x + nx;
		triangle[i].y = center.y + ny;
	}
}


void timer_callback() {
	if (!inited) return;

	draw_triangle(0x0);
	rotate_triangle(0.3f);
	draw_triangle(0xF);
}

void cmd_gfx(const char** args, int argc) {
	if (inited) {
		inited = 0;
		return;
	}

	triangle[0] = (point){100, 100};
	triangle[1] = (point){200, 200};
	triangle[2] = (point){300, 100};

	center = (point){200, 150};

	draw_triangle(0x0F);

	register_interrupt_handler(32, timer_callback);
	inited = 1;
}

void register_gfx_cmd(void) {
    register_command(
		"gfx", // name
		"gfx test", // desc
		0, // hidden
		cmd_gfx, // func
		0 // args
	);
}