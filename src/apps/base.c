
#include "apps/base.h"
#include "globals.h"

extern command_t __start_cmds;
extern command_t __stop_cmds;

command_t commands[MAX_COMMANDS];
int command_count = 0;

void register_command(const char* name, const char* description, int hidden, command_func_t func, int args) {
	if (command_count < MAX_COMMANDS) {
		commands[command_count].name = name;
		commands[command_count].description = description;
		commands[command_count].hidden = hidden;
		commands[command_count].func = func;
		commands[command_count].args = args;
		command_count++;
	}
}

void register_all_commands(void) {
	// printf("__start_cmds: %p\n__stop_cmds: %p\n", __start_cmds, __stop_cmds);
	command_t* cmd = &__start_cmds;
	while (cmd < &__stop_cmds) {
		// printf("registering command %s - %s\n", cmd->name, cmd->description);
		register_command(cmd->name, cmd->description, cmd->hidden, cmd->func, cmd->args);
		cmd++;
	}
}