#ifndef FAT32_FILE_H
#define FAT32_FILE_H

#include "globals.h"

#define FAT32_FAT_SECTOR_START 32
#define FAT32_MAX_FILES 128
#define FILE_TABLE_CLUSTER (FAT32_ROOT_CLUSTER) 

int fat32_file_make(const char* name);
int fat32_file_write_str(const char* name, const char* text);
int fat32_file_read(const char* name, char* output, nat32 maxlen);
int fat32_file_count();
const char* fat32_file_get_name(int id);
nat32 fat32_file_size(const char* name);
void fat32_fs_init();
#endif
