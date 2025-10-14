// commands base

#ifndef COMMAND_H
#define COMMAND_H

typedef void (*command_func_t)(const char* args);

typedef struct {
	const char* name;
	void (*func)(const char* args);
} command_t;

extern command_t commands[];

#endif
