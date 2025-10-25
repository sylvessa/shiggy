#include "globals.h"
#include "apps/base.h"
#include "graphics/main.h"
#include "lib/math.h"
#include "apps/gfx.h"
#include "graphics/3d.h"

typedef struct { int x, y; } point;

float angle = 0;
float speed = 20.0f;
int inited = 0;
mesh3d cube;

static point prev_proj[8];

static nat8 rainbow_step = 0;

static void update_rainbow_color() {
	nat8 r = 0, g = 0, b = 0;
	nat8 phase = rainbow_step % 192; // 3*64

	if(phase < 64) {
		r = 63 - phase;
		g = phase;
		b = 0;
	} else if(phase < 128) {
		r = 0;
		g = 127 - phase;
		b = phase - 64;
	} else {
		r = phase - 128;
		g = 0;
		b = 191 - phase;
	}

	rainbow_step++;

	out_byte(0x3C8, 7);
	out_byte(0x3C9, r);
	out_byte(0x3C9, g);
	out_byte(0x3C9, b);
}


void project_mesh(mesh3d *mesh, float scale, int cx, int cy, point *out){
	for (int i=0; i < mesh->vertex_count; i++) {
		project_point(mesh->vertices[i], &out[i].x, &out[i].y, scale, cx, cy);
	}
}

void gfx_app_timer_callback() {
	if (!inited) return;

	static nat32 last_tick = 0;
	nat32 now = 1;
	float dt = (now - last_tick) / 1000.0f;
	last_tick = now;

	for (int i = 0; i < cube.edge_count; i++) {
		int a = cube.edges[i][0];
		int b = cube.edges[i][1];
		gfx_draw_line(prev_proj[a].x, prev_proj[a].y, prev_proj[b].x, prev_proj[b].y, 0, 1);
	}

	point new_proj[8];
	//project_mesh(&cube, 5.0f, VGA_WIDTH/2, VGA_HEIGHT/2, new_proj);

	// angle += speed * dt;
	// if (angle > 6.283185f) angle -= 6.283185f;

	//rotate_mesh(&cube, angle*0.5f, angle, angle*0.25f);
	project_mesh(&cube, 5.0f, VGA_WIDTH/2, VGA_HEIGHT/2, new_proj);

	update_rainbow_color();

	for (int i = 0; i < cube.edge_count; i++) {
		int a = cube.edges[i][0];
		int b = cube.edges[i][1];
		gfx_draw_line(new_proj[a].x, new_proj[a].y, new_proj[b].x, new_proj[b].y, 7, 1);
	}

	for (int i = 0; i < cube.vertex_count; i++) prev_proj[i] = new_proj[i];
}


void cmd_gfx(const char** args, int argc){
	if (inited) {
		point erase_proj[8];

		project_mesh(&cube, 5.0f, VGA_WIDTH/2, VGA_HEIGHT/2, erase_proj);

		for(int i=0; i < cube.edge_count; i++){
			int a = cube.edges[i][0];
			int b = cube.edges[i][1];
			gfx_draw_line(erase_proj[a].x, erase_proj[a].y, erase_proj[b].x, erase_proj[b].y, 0, 1);
		}

		inited = 0;
		return;
	}

	if(argc >= 1) speed = atof(args[0]);
	if(argc >= 2) cube = cube_mesh(atof(args[1]));

	else cube = cube_mesh(15.0f);
	
	project_mesh(&cube, 5.0f, VGA_WIDTH/2, VGA_HEIGHT/2, prev_proj);

	inited = 1;
}


void register_gfx_cmd(void){
	register_command(
		"gfx",
		"shows rotating 3d cube. gfx <speed> <size>",
		0,
		cmd_gfx,
		1
	);
}
