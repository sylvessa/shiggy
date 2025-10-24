#ifndef GLOBALS_H
#define GLOBALS_H

#include "types.h"
#include "lib/pio.h"
#include "cpu/isr.h"
#include "lib/memory.h"
#include "lib/stdarg.h"
#include "fs/fat32.h"
#include "drivers/ata.h"
#include "lib/string.h"
#include "drivers/vga.h"
#include "lib/rand.h"
#include "drivers/cmos.h"

extern char kernel_start_addr;
extern char kernel_end_addr;
extern void *__free_memory_start;
extern bool gui_mode;
extern char* current_dir;  // defualts to / (its a string)
extern int current_dir_cluster; // cluster id


#endif