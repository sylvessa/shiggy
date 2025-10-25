#include "globals.h"
#include "apps/base.h"

static nat8 rand_color() {
	return (rand() % 15) + 1;
}

static void swap_commands(command_t* a, command_t* b) {
	command_t tmp = *a;
	*a = *b;
	*b = tmp;
}

static int command_cmp(const command_t* a, const command_t* b) {
	return strcmp(a->name, b->name);
}

static void qsort_commands(command_t* arr, int left, int right) {
	if (left >= right) return;

	int i = left, j = right;
	command_t pivot = arr[(left + right) / 2];

	while (i <= j) {
		while (command_cmp(&arr[i], &pivot) < 0) i++;
		while (command_cmp(&arr[j], &pivot) > 0) j--;
		
		if (i <= j) {
			swap_commands(&arr[i], &arr[j]);
			i++; j--;
		}
	}

	if (left < j) qsort_commands(arr, left, j);
	if (i < right) qsort_commands(arr, i, right);
}

static void cmd_help(const char** args, int argc) {
	print("this is an experimental os written by \\5xsylvessa\\x\n\n");
	print("written in C and a bit of x86 assembly\n");
	print("bootloader and everything was made by me.\n");
	print("\ncommands:\n"); 

	qsort_commands(commands, 0, command_count - 1);

	for (int i = 0; i < command_count; i++) { 
		if (!commands[i].hidden) { 
			nat8 c = rand_color();
			for (const char* s = commands[i].name; *s; s++) {
				print_char(*s, c, 0x00);
			}
			print(" - "); 
			print((char*)commands[i].description); 
			print("\n"); 
		} 
	}

	print("\n");
}


void register_help_cmd(void) {
    register_command(
		"help", // name
		"shows this help message", // desc
		0, // hidden
		cmd_help, // func
		0 // args
	);
}