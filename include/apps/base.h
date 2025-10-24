#ifndef COMMAND_H
#define COMMAND_H

typedef enum {
	ARG_NONE,
	ARG_STRING,
	ARG_NUMBER,
	ARG_MULTIPLE
} arg_type_t;

typedef void (*command_func_t)(const char** args, int argc);

typedef struct {
	const char* name;
	const char* description;
	bool hidden;
	command_func_t func;
	arg_type_t args;
} command_t;

#define MAX_COMMANDS 64

extern command_t commands[];
extern int command_count;

void register_command(const char* name, const char* description, bool hidden, command_func_t func, arg_type_t arg_type);
void list_commands();
void register_all_commands_from_section();

struct command_reg {
	const char* name;
	const char* description;
	bool hidden;
	command_func_t func;
	arg_type_t args;
};

extern struct command_reg* __start_cmds;
extern struct command_reg* __stop_cmds;

#endif
