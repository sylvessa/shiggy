#ifndef FAT32_H
#define FAT32_H

#include "globals.h"

#define SECTOR_SIZE 512
#define FAT32_ROOT_CLUSTER 2

typedef struct {
	nat8 name[11];
	nat8 attr;
	nat8 reserved;
	nat8 create_time_tenth;
	nat16 create_time;
	nat16 create_date;
	nat16 last_access_date;
	nat16 first_cluster_high;
	nat16 write_time;
	nat16 write_date;
	nat16 first_cluster_low;
	nat32 file_size;
} __attribute__((packed)) fat32_dir_entry_t;

void fat32_fs_init();
nat8 fat32_create_file(const char *name, const char *content);
nat8 fat32_read_file(const char *name, char *buffer, nat32 max_size);
nat32 fat32_file_size(const char* name);
const char* fat32_file_get_name(nat32 index);
nat32 fat32_file_count();

#endif
