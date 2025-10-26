#include "apps/base.h"
#include "drivers/pcspeaker.h"
#include "globals.h"

static void cmd_beep(const char** args, int argc) {
	if (argc < 1) {
		print("usage: beep <freq>\n");
		return;
	}

	int n = 0;
	for (int i = 0; args[0][i] != '\0'; i++) {
		if (args[0][i] < '0' || args[0][i] > '9') {
			print("argument must be a positive number\n");
			return;
		}
		n = n * 10 + (args[0][i] - '0');
	}

	pc_speaker_beep(n, 1000);
}

command_t beep_cmd __attribute__((section(".cmds"))) = {
	.magic = COMMAND_MAGIC,
	.name = "beep",
	.description = "beep",
	.hidden = 0,
	.func = cmd_beep,
	.args = 1};