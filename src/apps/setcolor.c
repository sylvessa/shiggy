#include "apps/base.h"
#include "globals.h"

static void cmd_setcolor(const char** args, int argc) {
	if (argc < 4) {
		print("usage: setcolor <color 0-15> <r 0-255> <g 0-255> <b 0-255>\n");
		return;
	}

	int color = 0, r = 0, g = 0, b = 0;

	for (int i = 0; args[0][i]; i++) {
		if (args[0][i] < '0' || args[0][i] > '9') {
			print("color must be a number 0-15\n");
			return;
		}
		color = color * 10 + (args[0][i] - '0');
	}
	if (color < 0 || color > 15) {
		print("color must be between 0 and 15\n");
		return;
	}

	const char* vals[3] = {args[1], args[2], args[3]};
	int* out[3] = {&r, &g, &b};

	for (int j = 0; j < 3; j++) {
		for (int i = 0; vals[j][i]; i++) {
			if (vals[j][i] < '0' || vals[j][i] > '9') {
				print("r, g, b must be numbers 0-255\n");
				return;
			}
			*out[j] = *out[j] * 10 + (vals[j][i] - '0');
		}
		if (*out[j] < 0 || *out[j] > 255) {
			print("r, g, b values must be between 0 and 255\n");
			return;
		}
	}

	set_color(color, r, g, b);
	printf("set color %d to rgb(%d,%d,%d)\n", color, r, g, b);
}

command_t setcolor_cmd __attribute__((section(".cmds"))) = {
	.magic = COMMAND_MAGIC,
	.name = "setcolor",
	.description = "sets a color in the palette",
	.hidden = 0,
	.func = cmd_setcolor,
	.args = 4};