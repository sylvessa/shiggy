#include "graphics/3d.h"
#include "globals.h"

mesh3d cube_mesh(float size) {
	mesh3d cube;
	static vec3 verts[8];
	static int ed[12][2] = {
		{0, 1}, {1, 3}, {3, 2}, {2, 0}, // bottom
		{4, 5},
		{5, 7},
		{7, 6},
		{6, 4}, // top
		{0, 4},
		{1, 5},
		{2, 6},
		{3, 7} // verticals
	};

	float hs = size / 2.0f;
	verts[0] = (vec3){-hs, -hs, -hs};
	verts[1] = (vec3){hs, -hs, -hs};
	verts[2] = (vec3){-hs, hs, -hs};
	verts[3] = (vec3){hs, hs, -hs};
	verts[4] = (vec3){-hs, -hs, hs};
	verts[5] = (vec3){hs, -hs, hs};
	verts[6] = (vec3){-hs, hs, hs};
	verts[7] = (vec3){hs, hs, hs};

	cube.vertices = verts;
	cube.vertex_count = 8;
	cube.edges = ed;
	cube.edge_count = 12;

	return cube;
}

void rotate_mesh(mesh3d* mesh, float rx, float ry, float rz) {
	for (int i = 0; i < mesh->vertex_count; i++) {
		vec3 v = mesh->vertices[i];

		// rotate X
		float y = v.y * cos(rx) - v.z * sin(rx);
		float z = v.y * sin(rx) + v.z * cos(rx);
		v.y = y;
		v.z = z;

		// rotate Y
		float x = v.x * cos(ry) + v.z * sin(ry);
		z = -v.x * sin(ry) + v.z * cos(ry);
		v.x = x;
		v.z = z;

		// rotate Z
		x = v.x * cos(rz) - v.y * sin(rz);
		y = v.x * sin(rz) + v.y * cos(rz);
		v.x = x;
		v.y = y;

		mesh->vertices[i] = v;
	}
}

void project_point(vec3 v, int* sx, int* sy, float scale, int cx, int cy) {
	float distance = 50.0f; // distance from camera
	float factor = distance / (distance - v.z);
	*sx = (int)(v.x * scale * factor) + cx;
	*sy = (int)(v.y * scale * factor) + cy;
}

void draw_mesh(mesh3d* mesh, float scale, int cx, int cy, nat8 color) {
	for (int i = 0; i < mesh->edge_count; i++) {
		vec3 a = mesh->vertices[mesh->edges[i][0]];
		vec3 b = mesh->vertices[mesh->edges[i][1]];

		int x0, y0, x1, y1;
		project_point(a, &x0, &y0, scale, cx, cy);
		project_point(b, &x1, &y1, scale, cx, cy);

		gfx_draw_line(x0, y0, x1, y1, color, 4);
	}
}

vec3 cube_center(mesh3d* mesh) {
	vec3 center = {0, 0, 0};
	if (is_mesh_empty(mesh)) return center;

	for (int i = 0; i < mesh->vertex_count; i++) {
		center.x += mesh->vertices[i].x;
		center.y -= mesh->vertices[i].y;
		center.z += mesh->vertices[i].z;
	}
	center.x /= mesh->vertex_count;
	center.y /= mesh->vertex_count;
	center.z /= mesh->vertex_count;
	return center;
}

int is_mesh_empty(mesh3d* m) {
	return m == NULL || m->vertices == NULL || m->vertex_count == 0;
}

void translate_mesh(mesh3d* mesh, float tx, float ty, float tz) {
	if (is_mesh_empty(mesh)) return;

	for (int i = 0; i < mesh->vertex_count; i++) {
		mesh->vertices[i].x += tx;
		mesh->vertices[i].y += ty;
		mesh->vertices[i].z += tz;
	}
}
