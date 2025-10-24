#include "globals.h"
#include "apps/base.h"
#include "drivers/pcspeaker.h"

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

void register_beep_cmd(void) {
    register_command(
		"beep", // name
		"beep", // desc
		0, // hidden
		cmd_beep, // func
		1 // args
	);
}