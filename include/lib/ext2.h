#ifndef EXT2_H
#define EXT2_H

#include "types.h"

#define EXT2_SUPERBLOCK_OFFSET 1024
#define EXT2_SUPER_MAGIC 0xEF53

typedef struct {
    nat32 s_inodes_count;
    nat32 s_blocks_count;
    nat32 s_r_blocks_count;
    nat32 s_free_blocks_count;
    nat32 s_free_inodes_count;
    nat32 s_first_data_block;
    nat32 s_log_block_size;
    nat32 s_log_frag_size;
    nat32 s_blocks_per_group;
    nat32 s_frags_per_group;
    nat32 s_inodes_per_group;
    nat32 s_mtime;
    nat32 s_wtime;
    nat16 s_mnt_count;
    nat16 s_max_mnt_count;
    nat16 s_magic;
    nat16 s_state;
    nat16 s_errors;
    nat16 s_minor_rev_level;
    nat32 s_lastcheck;
    nat32 s_checkinterval;
    nat32 s_creator_os;
    nat32 s_rev_level;
    /* only fields we need */
    nat32 s_first_ino; /* first non-reserved inode */
} ext2_super_t;

typedef struct {
    nat32 inode;
    nat16 rec_len;
    nat8 name_len;
    nat8 file_type;
    /* name follows */
} ext2_dir_entry_t;

void ext2_init(nat32 fs_phys_addr, nat16 fs_sectors);
int ext2_list_root(void);

#endif
