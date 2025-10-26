#include "apps/base.h"
#include "apps/gfx.h"
#include "globals.h"
#include "graphics/3d.h"
#include "graphics/main.h"
#include "lib/math.h"

void cmd_ctranslate(const char** args, int argc) {
	if (argc >= 1) {
		float xamt = atof(args[0]);
		float yamt = (argc >= 2) ? atof(args[1]) : 0;
		float zamt = (argc >= 3) ? atof(args[2]) : 0;

		if (is_mesh_empty(&cube)) {
			print("init a cube with gfx command\n");
			return;
		}

		float steps = 50.0f;
		float dx = xamt / steps;
		float dy = yamt / steps;
		float dz = zamt / steps;

		int i = 0;
		while (i < (int)steps) {
			translate_mesh(&cube, dx, dy, dz);
			sleep_ms(5);
			i++;
		}

	} else {
		print("args: ctranslate <x amt> [y amt] [z amt]\n");
	}
}

command_t ctranslate_cmd __attribute__((section(".cmds"))) = {
	.magic = COMMAND_MAGIC,
	.name = "ctranslate",
	.description = "translates (moves) a cube",
	.hidden = 0,
	.func = cmd_ctranslate,
	.args = 3};