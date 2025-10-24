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

extern char kernel_start_addr;
extern char kernel_end_addr;
extern void *__free_memory_start;
extern bool gui_mode;

#endif