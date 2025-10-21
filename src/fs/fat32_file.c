#include "globals.h"
#include "fs/fat32_file.h"
#include "drivers/screen.h"
#include "lib/string.h"
#include "lib/memory.h"
#include "fs/fat32.h"

typedef struct {
	char name[12];
	nat32 start_cluster;
	nat32 size;
} FileEntry;

static FileEntry file_table[FAT32_MAX_FILES];
static int file_count_var = 0;
static nat32 fat_table[4096]; // simple in-memory FAT
static nat32 next_free_cluster = FILE_TABLE_CLUSTER + 1; // next free cluster after file table

static nat32 alloc_cluster(nat32 prev) {
	nat32 cluster = next_free_cluster++;
	fat_table[cluster] = 0x0FFFFFFF; // end-of-chain
	if (prev != 0) fat_table[prev] = cluster;
	return cluster;
}

static void write_cluster(nat32 cluster, const nat8* data) {
	for (nat32 sector = 0; sector < FAT32_SECTORS_PER_CLUSTER; sector++) {
		nat32 lba = FAT32_DATA_START + (cluster - FAT32_ROOT_CLUSTER) * FAT32_SECTORS_PER_CLUSTER + sector;
		fat32_write_sector(lba, (nat8*)(data + sector * SECTOR_SIZE));
	}
}

static void read_cluster(nat32 cluster, nat8* data) {
	for (nat32 sector = 0; sector < FAT32_SECTORS_PER_CLUSTER; sector++) {
		nat32 lba = FAT32_DATA_START + (cluster - FAT32_ROOT_CLUSTER) * FAT32_SECTORS_PER_CLUSTER + sector;
		fat32_read_sector(lba, data + sector * SECTOR_SIZE);
	}
}

static void save_file_table() {
	nat8* buffer = calloc(FAT32_CLUSTER_SIZE, 1);
	memcpy(buffer, (const byte*)file_table, sizeof(file_table));
	write_cluster(FILE_TABLE_CLUSTER, buffer);
	free(buffer);
}

void load_file_table() {
	nat8* buffer = calloc(FAT32_CLUSTER_SIZE, 1);
	read_cluster(FILE_TABLE_CLUSTER, buffer);
	memcpy((byte*)file_table, buffer, sizeof(file_table));
	free(buffer);

	file_count_var = 0;
	for (int i = 0; i < FAT32_MAX_FILES; i++)
		if (file_table[i].name[0] != '\0')
			file_count_var++;
}

int fat32_file_make(const char* name) {
	if (file_count_var >= FAT32_MAX_FILES) return -1;

	FileEntry* fe = &file_table[file_count_var++];
	memmove((byte*)fe, 0, sizeof(FileEntry));
	strncpy(fe->name, name, 11);
	fe->name[11] = '\0'; // ensure null-terminated
	fe->start_cluster = alloc_cluster(0);
	fe->size = 0;

	nat8* buffer = calloc(FAT32_CLUSTER_SIZE, 1);
	write_cluster(fe->start_cluster, buffer);
	free(buffer);

	save_file_table();
	return 0;
}

FileEntry* fat32_find_file(const char* name) {
	for (int i = 0; i < file_count_var; i++)
		if (strncmp(file_table[i].name, name, 11) == 0) return &file_table[i];
	return NULL;
}

int fat32_file_write_str(const char* name, const char* text) {
	FileEntry* fe = fat32_find_file(name);
	if (!fe) return -1;

	nat32 len = (nat32)strlen((char*)text);
	nat32 remaining = len;
	nat32 cluster = fe->start_cluster;
	const nat8* ptr = (const nat8*)text;

	while (remaining > 0) {
		nat8* buffer = calloc(FAT32_CLUSTER_SIZE, 1);
		nat32 chunk = remaining > FAT32_CLUSTER_SIZE ? FAT32_CLUSTER_SIZE : remaining;
		memcpy(buffer, (const byte*)ptr, chunk);
		write_cluster(cluster, buffer);
		free(buffer);

		ptr += chunk;
		remaining -= chunk;

		if (remaining > 0) cluster = alloc_cluster(cluster);
	}

	fe->size = len;
	save_file_table();
	return 0;
}

int fat32_file_read(const char* name, char* output, nat32 maxlen) {
	FileEntry* fe = fat32_find_file(name);
	if (!fe) return -1;

	nat32 remaining = fe->size;
	nat32 cluster = fe->start_cluster;
	nat32 out_offset = 0;

	while (remaining > 0 && maxlen > 0) {
		nat8* buffer = calloc(FAT32_CLUSTER_SIZE, 1);
		read_cluster(cluster, buffer);

		nat32 chunk = remaining > FAT32_CLUSTER_SIZE ? FAT32_CLUSTER_SIZE : remaining;
		if (chunk > maxlen) chunk = maxlen;

		memcpy((byte*)(output + out_offset), buffer, chunk);
		free(buffer);

		out_offset += chunk;
		remaining -= chunk;
		maxlen -= chunk;

		if (remaining > 0) cluster = fat_table[cluster];
	}

	return 0;
}

int fat32_file_count() { return file_count_var; }

const char* fat32_file_get_name(int id) {
	if (id < 0 || id >= file_count_var) return NULL;
	return file_table[id].name;
}

nat32 fat32_file_size(const char* name) {
	FileEntry* fe = fat32_find_file(name);
	if (!fe) return 0;
	return fe->size;
}

void fat32_fs_init() {
	if (!fat32_is_formatted()) {
		fat32_format();

		fat32_file_make("READ.TXT");
		fat32_file_write_str("READ.TXT", "simply amazing");
	} else {
		load_file_table();
	}
}