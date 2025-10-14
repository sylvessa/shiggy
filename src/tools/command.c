#include "globals.h"
#include "tools/command.h"

extern void cmd_help(const char* args);
extern void cmd_test(const char* args);
extern void cmd_clear(const char* args);

command_t commands[] = {
	{"help", cmd_help},
	{"test", cmd_test},
	{"clear", cmd_clear}
};
