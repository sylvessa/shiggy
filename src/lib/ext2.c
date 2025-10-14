#include "globals.h"
#include "lib/ext2.h"

static byte *fs_base = NULL;
static nat32 fs_size = 0;
static nat32 block_size = 1024;
static ext2_super_t *super = NULL;

// helper to read from physical memory mapped by bootloader
static inline void *phys_ptr(nat32 phys_addr) {
    return (void *)phys_addr;
}

void ext2_init(nat32 fs_phys_addr, nat16 fs_sectors) {
    fs_base = (byte *)phys_ptr(fs_phys_addr);
    fs_size = (nat32)fs_sectors * 512;

    super = (ext2_super_t *)(fs_base + EXT2_SUPERBLOCK_OFFSET);
    if (super->s_magic != EXT2_SUPER_MAGIC) {
        print("ext2: invalid magic or no fs present\n");
        super = NULL;
        return;
    }

    block_size = 1024 << super->s_log_block_size;
    print("ext2: initialized\n");
}

static byte *block_ptr(nat32 block) {
    return fs_base + (block * block_size);
}

int ext2_list_root(void) {
    if (!super) return -1;

    // root inode is 2
    nat32 root_inode = 2;
    byte *b = block_ptr(5);
    // inode size default 128, root inode is the second entry
    byte *inode_ptr = b + 128 * (root_inode - 1);

    // direct block pointers start at offset 40 (for ext2 inode)
    nat32 *blocks = (nat32 *)(inode_ptr + 40);
    nat32 block0 = blocks[0];
    if (block0 == 0) {
        print("ext2: root has no blocks\n");
        return -1;
    }

    byte *dir_block = block_ptr(block0);
    nat32 offset = 0;
    while (offset < block_size) {
        ext2_dir_entry_t *ent = (ext2_dir_entry_t *)(dir_block + offset);
        if (ent->inode == 0) break;

        char name[256];
        for (int i = 0; i < ent->name_len && i < 255; i++) {
            name[i] = *((char *)(dir_block + offset + sizeof(ext2_dir_entry_t) + i));
        }
        name[ent->name_len] = '\0';

        print(name);
        print("\n");

        offset += ent->rec_len;
        if (ent->rec_len == 0) break;
    }

    return 0;
}
