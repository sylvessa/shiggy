#include "globals.h"

#define ROOT_DIR_LBA 2
#define FIRST_FILE_CLUSTER 3
#define MAX_ROOT_ENTRIES 16

nat8 fat32_log = 1;

static nat32 fat_start_lba;
static nat32 cluster2_lba;
static nat32 sectors_per_fat;
static nat32 total_clusters;
static nat32 next_free_cluster;

static nat8 fat[SECTOR_SIZE*8];
static fat32_dir_entry_t root_dir[MAX_ROOT_ENTRIES];

static void write_sector_lba(nat32 lba, const nat8 *buffer) {
    ata_write_sector(lba, buffer);
}

static void read_sector_lba(nat32 lba, nat8 *buffer) {
    ata_read_sector(lba, buffer);
}

static nat32 cluster_to_lba(nat32 cluster) {
    return cluster2_lba + (cluster - FIRST_FILE_CLUSTER);
}

static void init_fat() {
    for (nat32 i = 0; i < total_clusters; i++) fat[i] = 0;
    fat[0] = 0xF8; fat[1] = 0xFF; fat[2] = 0xFF;
}

static void write_root_dir() {
    nat8 buffer[SECTOR_SIZE];
    for (int i = 0; i < SECTOR_SIZE; i++) buffer[i] = 0;

    for (int i = 0; i < MAX_ROOT_ENTRIES; i++) {
        if (root_dir[i].name[0] == 0 || root_dir[i].name[0] == 0xE5) continue;

        int offset = i * 32;
        
        for (int j = 0; j < 11; j++) buffer[offset + j] = root_dir[i].name[j];
        buffer[offset + 11] = root_dir[i].attr;
        
        buffer[offset + 20] = root_dir[i].first_cluster_high & 0xFF;
        buffer[offset + 21] = (root_dir[i].first_cluster_high >> 8) & 0xFF;
        
        buffer[offset + 26] = root_dir[i].first_cluster_low & 0xFF;
        buffer[offset + 27] = (root_dir[i].first_cluster_low >> 8) & 0xFF;
        
        buffer[offset + 28] = root_dir[i].file_size & 0xFF;
        buffer[offset + 29] = (root_dir[i].file_size >> 8) & 0xFF;
        buffer[offset + 30] = (root_dir[i].file_size >> 16) & 0xFF;
        buffer[offset + 31] = (root_dir[i].file_size >> 24) & 0xFF;
    }

    write_sector_lba(ROOT_DIR_LBA, buffer);
}

static void load_root_dir() {
    nat8 buffer[SECTOR_SIZE];
    read_sector_lba(ROOT_DIR_LBA, buffer);

    for (int i = 0; i < MAX_ROOT_ENTRIES; i++) {
        int offset = i * 32;
        for (int j = 0; j < 11; j++) root_dir[i].name[j] = buffer[offset + j];
        root_dir[i].attr = buffer[offset + 11];
        root_dir[i].first_cluster_high = (nat16)buffer[offset + 20] | ((nat16)buffer[offset + 21] << 8);
        root_dir[i].first_cluster_low = (nat16)buffer[offset + 26] | ((nat16)buffer[offset + 27] << 8);
        root_dir[i].file_size = (nat32)buffer[offset + 28] | 
                              ((nat32)buffer[offset + 29] << 8) | 
                              ((nat32)buffer[offset + 30] << 16) | 
                              ((nat32)buffer[offset + 31] << 24);
    }

    next_free_cluster = FIRST_FILE_CLUSTER;
    for (int i = 0; i < MAX_ROOT_ENTRIES; i++) {
        if (root_dir[i].first_cluster_low >= next_free_cluster)
            next_free_cluster = root_dir[i].first_cluster_low + 1;
    }
}

static nat8 is_formatted() {
    nat8 buffer[SECTOR_SIZE];
    read_sector_lba(0, buffer);
    return (buffer[510] == 'S' && buffer[511] == 'H') ? 1 : 0;
}

static void format_fs() {
    nat8 buffer[SECTOR_SIZE];
    for (int i = 0; i < SECTOR_SIZE; i++) buffer[i] = 0;

    buffer[510] = 'S';
    buffer[511] = 'H';
    write_sector_lba(0, buffer);

    fat_start_lba = 1;
    sectors_per_fat = 1;
    cluster2_lba = FIRST_FILE_CLUSTER; // LBA of Cluster 3 data (LBA 3, from hexdump)
    total_clusters = 256;

    init_fat();

    for (int i = 0; i < SECTOR_SIZE; i++) buffer[i] = 0;
    buffer[0] = 0xF8; buffer[1] = 0xFF; buffer[2] = 0xFF;
    write_sector_lba(fat_start_lba, buffer);

    for (int i = 0; i < MAX_ROOT_ENTRIES; i++) root_dir[i].name[0] = 0;
    write_root_dir();

    next_free_cluster = FIRST_FILE_CLUSTER;
}

void fat32_fs_init() {
    fat_start_lba = 1;
    sectors_per_fat = 1;
    cluster2_lba = FIRST_FILE_CLUSTER; 
    total_clusters = 256;

    if (!is_formatted()) {
        format_fs();
    } else {
        load_root_dir();
    }
}

nat8 fat32_create_file(const char *name, const char *content) {
    for (int i = 0; i < MAX_ROOT_ENTRIES; i++) {
        if (root_dir[i].name[0] == 0 || root_dir[i].name[0] == 0xE5) {
            
            memset(&root_dir[i], 0, sizeof(fat32_dir_entry_t)); 

            strncpy((char*)root_dir[i].name, name, 11);
            root_dir[i].attr = 0x20;

            root_dir[i].first_cluster_high = 0; 

            root_dir[i].first_cluster_low = next_free_cluster;
            nat32 file_cluster = next_free_cluster;
            next_free_cluster++;

            nat32 sz = strlen(content);
            root_dir[i].file_size = sz;

            nat8 buffer[SECTOR_SIZE];
            for (int j = 0; j < SECTOR_SIZE; j++) buffer[j] = 0;
            for (nat32 j = 0; j < sz && j < SECTOR_SIZE; j++) buffer[j] = content[j];

            write_sector_lba(cluster_to_lba(file_cluster), buffer);
            write_root_dir();

            if (fat32_log) printf("[fat32] file '%s' written at cluster %d, size %d\n", name, file_cluster, sz);
            return 1;
        }
    }
    return 0;
}

nat8 fat32_read_file(const char *name, char *buffer, nat32 max_size) {
    for (int i = 0; i < MAX_ROOT_ENTRIES; i++) {
        if (strncmp((char*)root_dir[i].name, name, 11) == 0) {
            nat32 sz = root_dir[i].file_size;
            if (sz > max_size) sz = max_size;

            nat32 file_cluster = root_dir[i].first_cluster_low;

            nat8 temp[SECTOR_SIZE];
            read_sector_lba(cluster_to_lba(file_cluster), temp);

            for (nat32 j = 0; j < sz; j++) buffer[j] = temp[j];
            buffer[sz] = 0;
            return 1;
        }
    }
    return 0;
}

nat32 fat32_file_count() {
    nat32 count = 0;
    for (int i = 0; i < MAX_ROOT_ENTRIES; i++) {
        if (root_dir[i].name[0] != 0 && root_dir[i].name[0] != 0xE5) count++;
    }
    return count;
}

const char* fat32_file_get_name(nat32 index) {
    char* name = malloc(12);
    nat32 c = 0;
    for (int i = 0; i < MAX_ROOT_ENTRIES; i++) {
        if (root_dir[i].name[0] != 0 && root_dir[i].name[0] != 0xE5) {
            if (c == index) {
                for (int j = 0; j < 11; j++) name[j] = root_dir[i].name[j];
                name[11] = 0;
                return name;
            }
            c++;
        }
    }
    return 0;
}

nat32 fat32_file_size(const char* name) {
    for (int i = 0; i < MAX_ROOT_ENTRIES; i++) {
        if (strncmp((char*)root_dir[i].name, name, 11) == 0) return root_dir[i].file_size;
    }
    return 0;
}
