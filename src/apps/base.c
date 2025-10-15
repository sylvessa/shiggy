#include "globals.h"
#include "apps/base.h"

command_t commands[MAX_COMMANDS];
int command_count = 0;

void register_command(const char* name, const char* description, bool hidden, command_func_t func) {
	if (command_count < MAX_COMMANDS) {
		commands[command_count].name = name;
		commands[command_count].description = description;
		commands[command_count].hidden = hidden;
		commands[command_count].func = func;
		command_count++;
	}
}

void list_commands() {
	print("\nAvailable commands:\n");
	for (int i = 0; i < command_count; i++) {
		if (!commands[i].hidden) {
			print((char*)commands[i].name);
			print(" - ");
			print((char*)commands[i].description);
			print("\n");
		}
	}
}

void register_all_commands_from_section() {
	struct command_reg** ptr = (struct command_reg**) &__start_cmds;
	struct command_reg** end = (struct command_reg**) &__stop_cmds;

	while (ptr < end) {
		register_command((*ptr)->name, (*ptr)->description, (*ptr)->hidden, (*ptr)->func);
		ptr++;
	}
}

