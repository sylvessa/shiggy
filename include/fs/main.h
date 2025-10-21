#ifndef FS_H
#define FS_H

#include "fs/props.h"


void fs_init();
char* file_get_name(int id);
int file_count();
int file_make(char* name);
int file_size(char* name);
int file_read(char* filename, char* output);

#endif
