#ifndef COMMAND_H
#define COMMAND_H

#include <stdbool.h>

typedef void (*command_func_t)(const char* args);

typedef struct {
	const char* name;
	const char* description;
	bool hidden;
	command_func_t func;
} command_t;

#define MAX_COMMANDS 64

extern command_t commands[];
extern int command_count;

void register_command(const char* name, const char* description, bool hidden, command_func_t func);
void list_commands();
void register_all_commands_from_section();

// single definition of struct command_reg
struct command_reg {
	const char* name;
	const char* description;
	bool hidden;
	command_func_t func;
};

extern struct command_reg* __start_cmds;
extern struct command_reg* __stop_cmds;

#endif
