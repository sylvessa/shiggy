#ifndef FAT32_H
#define FAT32_H

#include "globals.h"

#define SECTOR_SIZE 512
#define FAT32_ROOT_CLUSTER 2
#define ROOT_DIR_LBA 2
#define FIRST_FILE_CLUSTER 3
#define MAX_ROOT_ENTRIES 16

#define FAT32_ATTR_DIRECTORY 0x10
#define FAT32_ATTR_ARCHIVE 0x20

typedef struct __attribute__((packed)) {
	nat8 name[16];
	nat8 attr;
	nat8 reserved[11];
	nat16 first_cluster;
	nat16 parent_cluster;
	nat32 file_size;
} fat32_dir_entry_t;

extern fat32_dir_entry_t root_dir[MAX_ROOT_ENTRIES];

void fat32_fs_init();
nat8 fat32_create_file(nat32 dir_cluster, const char* name, const char* content);
nat8 fat32_create_dir(nat32 dir_cluster, const char* name);
nat8 fat32_read_file(nat32 dir_cluster, const char* name, char* buffer, nat32 max_size);
nat32 fat32_file_size(nat32 dir_cluster, const char* name);
const char* fat32_file_get_name(nat32 dir_cluster, nat32 index);
nat32 fat32_file_count(nat32 dir_cluster);
nat32 fat32_dir_count(nat32 dir_cluster);
void fat32_dir_get_entry(nat32 dir_cluster, nat32 index, fat32_dir_entry_t* entry);
nat8 is_formatted();

#endif
