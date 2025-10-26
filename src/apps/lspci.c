#include "apps/base.h"
#include "globals.h"

static void cmd_lspci(const char** args, int argc) {
	show_pci_devices();
}

command_t lspci_cmd __attribute__((section(".cmds"))) = {
	.magic = COMMAND_MAGIC,
	.name = "lspci",
	.description = "shows pci info",
	.hidden = 0,
	.func = cmd_lspci,
	.args = 0};