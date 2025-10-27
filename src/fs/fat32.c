#include "fs/fat32.h"
#include "globals.h"

nat8 fat32_log = 0;

static nat32 next_free_cluster;
fat32_dir_entry_t root_dir[MAX_ROOT_ENTRIES];

static void write_sector_lba(nat32 lba, const nat8* buffer) {
	ata_write_sector(lba, buffer);
}

static void read_sector_lba(nat32 lba, nat8* buffer) {
	ata_read_sector(lba, buffer);
}

static nat32 cluster_to_lba(nat32 cluster) {
	int root_dir_sectors = (MAX_ROOT_ENTRIES * sizeof(fat32_dir_entry_t) + SECTOR_SIZE - 1) / SECTOR_SIZE;
	return ROOT_DIR_LBA + root_dir_sectors + (cluster - FIRST_FILE_CLUSTER);
}

static void write_root_dir() {
	int total_bytes = sizeof(fat32_dir_entry_t) * MAX_ROOT_ENTRIES;
	int sectors_needed = (total_bytes + SECTOR_SIZE - 1) / SECTOR_SIZE;

	if (fat32_log) printf("writing root_dir, %d entries, %d sectors\n", MAX_ROOT_ENTRIES, sectors_needed);

	for (int s = 0; s < sectors_needed; s++) {
		nat8 buffer[SECTOR_SIZE];
		memset(buffer, 0, SECTOR_SIZE);

		int non_empty_count = 0;

		for (int j = 0; j < MAX_ROOT_ENTRIES; j++) {
			int start_byte = j * sizeof(fat32_dir_entry_t);
			int sector_index = start_byte / SECTOR_SIZE;
			if (sector_index != s) continue;

			int offset_in_sector = start_byte % SECTOR_SIZE;
			memcpy(buffer + offset_in_sector, &root_dir[j], sizeof(fat32_dir_entry_t));

			if (root_dir[j].name[0] != 0) non_empty_count++;
		}

		write_sector_lba(ROOT_DIR_LBA + s, buffer);

		if (fat32_log) {
			printf("sector %d written, %d non-empty entries\n", ROOT_DIR_LBA + s, non_empty_count);
			for (int j = 0; j < MAX_ROOT_ENTRIES && non_empty_count > 0; j++) {
				int start_byte = j * sizeof(fat32_dir_entry_t);
				if (start_byte / SECTOR_SIZE != s) continue;
				if (root_dir[j].name[0] != 0) {
					printf("  entry '%s' cluster %d attr 0x%x size %d\n",
						   root_dir[j].name,
						   root_dir[j].first_cluster,
						   root_dir[j].attr,
						   root_dir[j].file_size);
				}
			}
		}
	}

	if (fat32_log) printf("finished writing root_dir\n");
}

static void load_root_dir() {
	int total_bytes = sizeof(fat32_dir_entry_t) * MAX_ROOT_ENTRIES;
	int sectors_needed = (total_bytes + SECTOR_SIZE - 1) / SECTOR_SIZE;

	if (fat32_log) printf("loading root_dir, %d sectors\n", sectors_needed);

	for (int s = 0; s < sectors_needed; s++) {
		nat8 buffer[SECTOR_SIZE];
		read_sector_lba(ROOT_DIR_LBA + s, buffer);

		for (int j = 0; j < MAX_ROOT_ENTRIES; j++) {
			int start_byte = j * sizeof(fat32_dir_entry_t);
			int sector_index = start_byte / SECTOR_SIZE;
			if (sector_index != s) continue;

			int offset_in_sector = start_byte % SECTOR_SIZE;
			memcpy(&root_dir[j], buffer + offset_in_sector, sizeof(fat32_dir_entry_t));

			if (fat32_log && root_dir[j].name[0] != 0) {
				printf("loaded entry '%s' cluster %d attr 0x%x size %d\n",
					   root_dir[j].name,
					   root_dir[j].first_cluster,
					   root_dir[j].attr,
					   root_dir[j].file_size);
			}
		}
	}

	// fix next_free_cluster
	next_free_cluster = FIRST_FILE_CLUSTER;
	for (int i = 0; i < MAX_ROOT_ENTRIES; i++) {
		if (root_dir[i].name[0] != 0 && root_dir[i].first_cluster >= next_free_cluster) {
			next_free_cluster = root_dir[i].first_cluster + 1;
		}
	}

	if (fat32_log) printf("next free cluster: %d\n", next_free_cluster);
}

nat8 is_formatted() {
	nat8 buffer[SECTOR_SIZE];
	read_sector_lba(0, buffer);
	return (buffer[510] == 'S' && buffer[511] == 'H') ? 1 : 0;
}

static void format_fs() {
	nat8 buffer[SECTOR_SIZE];
	for (int i = 0; i < SECTOR_SIZE; i++)
		buffer[i] = 0;

	buffer[510] = 'S';
	buffer[511] = 'H';
	write_sector_lba(0, buffer);

	for (int i = 0; i < MAX_ROOT_ENTRIES; i++)
		root_dir[i].name[0] = 0;
	write_root_dir();

	next_free_cluster = FIRST_FILE_CLUSTER;
}

void fat32_fs_init() {
	if (!is_formatted())
		format_fs();
	else
		load_root_dir();
}

nat8 fat32_create_file(nat32 dir_cluster, const char* name, const char* content) {
	for (int i = 0; i < MAX_ROOT_ENTRIES; i++) {
		if (root_dir[i].name[0] == 0) {
			memset(&root_dir[i], 0, sizeof(fat32_dir_entry_t));

			strncpy((char*)root_dir[i].name, name, 255);
			root_dir[i].name[255] = 0;

			root_dir[i].attr = FAT32_ATTR_ARCHIVE;
			next_free_cluster++;
			root_dir[i].first_cluster = next_free_cluster;
			root_dir[i].parent_cluster = dir_cluster;
			root_dir[i].file_size = strlen(content);

			// write content sector(s)
			nat32 cluster_lba = cluster_to_lba(root_dir[i].first_cluster);
			nat8 fbuf[SECTOR_SIZE];
			memset(fbuf, 0, SECTOR_SIZE);
			strncpy((char*)fbuf, content, SECTOR_SIZE - 1);
			write_sector_lba(cluster_lba, fbuf);

			write_root_dir();
			return 1;
		}
	}
	return 0;
}

nat8 fat32_create_dir(nat32 dir_cluster, const char* name) {
	for (int i = 0; i < MAX_ROOT_ENTRIES; i++) {
		if (root_dir[i].name[0] == 0) {
			memset(&root_dir[i], 0, sizeof(fat32_dir_entry_t));

			strncpy((char*)root_dir[i].name, name, 255);
			root_dir[i].name[255] = 0;

			next_free_cluster++;
			root_dir[i].attr = FAT32_ATTR_DIRECTORY;
			root_dir[i].first_cluster = next_free_cluster;
			root_dir[i].parent_cluster = dir_cluster;
			root_dir[i].file_size = 0;

			write_root_dir();
			return 1;
		}
	}
	return 0;
}

nat8 fat32_read_file(nat32 dir_cluster, const char* name, char* buffer, nat32 max_size) {
	for (int i = 0; i < MAX_ROOT_ENTRIES; i++) {
		if (root_dir[i].name[0] == 0) continue;
		if (root_dir[i].parent_cluster != dir_cluster) continue;
		if (strcmp((char*)root_dir[i].name, name) == 0) {
			nat32 sz = root_dir[i].file_size;
			if (sz > max_size - 1) sz = max_size - 1;

			nat8 fbuf[SECTOR_SIZE];
			read_sector_lba(cluster_to_lba(root_dir[i].first_cluster), fbuf);
			strncpy(buffer, (char*)fbuf, sz);
			buffer[sz] = 0;
			return 1;
		}
	}
	return 0;
}

nat32 fat32_file_count(nat32 dir_cluster) {
	nat32 count = 0;
	for (int i = 0; i < MAX_ROOT_ENTRIES; i++)
		if (root_dir[i].name[0] != 0 && !(root_dir[i].attr & FAT32_ATTR_DIRECTORY) && root_dir[i].parent_cluster == dir_cluster) count++;
	return count;
}

nat32 fat32_dir_count(nat32 dir_cluster) {
	nat32 count = 0;
	for (int i = 0; i < MAX_ROOT_ENTRIES; i++)
		if (root_dir[i].name[0] != 0 && (root_dir[i].attr & FAT32_ATTR_DIRECTORY) && root_dir[i].parent_cluster == dir_cluster) count++;
	return count;
}

void fat32_dir_get_entry(nat32 dir_cluster, nat32 index, fat32_dir_entry_t* entry) {
	nat32 c = 0;
	for (int i = 0; i < MAX_ROOT_ENTRIES; i++) {
		if (root_dir[i].name[0] == 0) continue;
		if (root_dir[i].parent_cluster != dir_cluster) continue;
		if (c == index) {
			memcpy(entry, &root_dir[i], sizeof(fat32_dir_entry_t));
			return;
		}
		c++;
	}
}

nat32 fat32_file_size(nat32 dir_cluster, const char* name) {
	for (int i = 0; i < MAX_ROOT_ENTRIES; i++) {
		if (root_dir[i].name[0] == 0) continue;
		if (root_dir[i].parent_cluster != dir_cluster) continue;
		if (strcmp((char*)root_dir[i].name, name) == 0) return root_dir[i].file_size;
	}
	return 0;
}

const char* fat32_file_get_name(nat32 dir_cluster, nat32 index) {
	static char name[256];
	nat32 c = 0;
	for (int i = 0; i < MAX_ROOT_ENTRIES; i++) {
		if (root_dir[i].name[0] == 0) continue;
		if (root_dir[i].parent_cluster != dir_cluster) continue;
		if (c == index) {
			strncpy(name, (char*)root_dir[i].name, 255);
			name[255] = 0;
			return name;
		}
		c++;
	}
	return 0;
}

nat8 fat32_delete_file(nat32 dir_cluster, const char* name) {
	for (int i = 0; i < MAX_ROOT_ENTRIES; i++) {
		if (root_dir[i].name[0] == 0) continue;
		if (root_dir[i].parent_cluster != dir_cluster) continue;
		if (strcmp((char*)root_dir[i].name, name) == 0) {
			if (root_dir[i].attr & FAT32_ATTR_DIRECTORY) {
				// cannot delete directories yet
				return 0;
			}
			root_dir[i].name[0] = 0;
			root_dir[i].first_cluster = 0;
			root_dir[i].file_size = 0;
			write_root_dir();
			return 1;
		}
	}
	return 0;
}
