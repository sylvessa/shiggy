#ifndef GRAPHICS_3D_H
#define GRAPHICS_3D_H

#include "globals.h"
#include "graphics/main.h"

typedef struct {
	float x, y, z;
} vec3;

typedef struct {
	vec3 *vertices;
	int vertex_count;
	int (*edges)[2];  // array of vertex index pairs
	int edge_count;
} mesh3d;

mesh3d cube_mesh(float size);
void rotate_mesh(mesh3d *mesh, float rx, float ry, float rz);
void project_point(vec3 v, int *sx, int *sy, float scale, int cx, int cy);
void draw_mesh(mesh3d *mesh, float scale, int cx, int cy, nat8 color);
int is_mesh_empty(mesh3d *m);
void translate_mesh(mesh3d *mesh, float tx, float ty, float tz);

#endif
