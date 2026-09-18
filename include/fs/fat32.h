#ifndef FAT32_H
#define FAT32_H

#include "globals.h"

#define SECTOR_SIZE 512
#define FAT32_ROOT_CLUSTER 2

// FAT32 directory entry attribute bits
#define FAT32_ATTR_READ_ONLY 0x01
#define FAT32_ATTR_HIDDEN 0x02
#define FAT32_ATTR_SYSTEM 0x04
#define FAT32_ATTR_VOLUME_ID 0x08
#define FAT32_ATTR_DIRECTORY 0x10
#define FAT32_ATTR_ARCHIVE 0x20
// ATTR_READ_ONLY | ATTR_HIDDEN | ATTR_SYSTEM | ATTR_VOLUME_ID = 0x0F (LFN marker)
#define FAT32_ATTR_LFN 0x0F

#define FAT32_DELETED 0xE5
#define FAT32_ENTRY_END 0x00
#define FAT32_EOC 0x0FFFFFFF
typedef struct __attribute__((packed)) {
	nat8 name[8];
	nat8 ext[3];
	nat8 attr;
	nat8 nt_reserved;
	nat8 ctime_tenth;
	nat16 ctime;
	nat16 cdate;
	nat16 adate;
	nat16 first_cluster_hi;
	nat16 wtime;
	nat16 wdate;
	nat16 first_cluster_lo;
	nat32 file_size;
} fat32_dir_entry_t;
typedef struct {
	char name[256];
	nat8 attr;
	nat32 first_cluster;
	nat32 file_size;
} fat32_entry_info_t;

void fat32_fs_init();
nat8 is_hdd_present();
nat8 is_formatted();

nat8 fat32_create_file(nat32 dir_cluster, const char* name, const char* content);
nat8 fat32_create_dir(nat32 dir_cluster, const char* name);
nat8 fat32_read_file(nat32 dir_cluster, const char* name, char* buffer, nat32 max_size);
nat32 fat32_file_size(nat32 dir_cluster, const char* name);
nat8 fat32_delete_file(nat32 dir_cluster, const char* name);

nat32 fat32_file_count(nat32 dir_cluster);
nat32 fat32_dir_count(nat32 dir_cluster);
nat8 fat32_dir_get_entry(nat32 dir_cluster, nat32 index, fat32_entry_info_t* out);
nat8 fat32_dir_parent(nat32 dir_cluster, nat32* parent_out);

#endif