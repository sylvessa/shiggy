#ifndef COMMAND_H
#define COMMAND_H
#include "globals.h"

typedef void (*command_func_t)(const char** args, int argc);

typedef struct {
	nat32 magic; // magic number to validate command
	const char* name;
	const char* description;
	int hidden;
	command_func_t func;
	int args;
} command_t;

#define COMMAND_MAGIC 0x434D444D

#define MAX_COMMANDS 128
extern command_t commands[MAX_COMMANDS];
extern int command_count;

void register_command(const char* name, const char* description, int hidden, command_func_t func, int args);
void register_all_commands(void);

#endif
