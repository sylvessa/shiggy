
#include "globals.h"
#include "apps/base.h"

command_t commands[MAX_COMMANDS];
int command_count = 0;

extern void register_colors_cmd(void);
extern void register_test_cmd(void);
extern void register_mk_cmd(void);
extern void register_ls_cmd(void);
extern void register_help_cmd(void);
extern void register_gfx_cmd(void);
extern void register_diskinfo_cmd(void);
extern void register_clear_cmd(void);
extern void register_cat_cmd(void);
extern void register_time_cmd(void);
extern void register_gui_cmd(void);
extern void register_mkdir_cmd(void);
extern void register_cd_cmd(void);


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
    register_colors_cmd();
	register_test_cmd();
	register_mk_cmd();
	register_ls_cmd();
	register_help_cmd();
	register_gfx_cmd();
	register_diskinfo_cmd();
	register_clear_cmd();
	register_cat_cmd();
	register_time_cmd();
	register_gui_cmd();
	register_mkdir_cmd();
	register_cd_cmd();
}