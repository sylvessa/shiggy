#include "globals.h"

int findex_end = 0;
File* findex[FS_MAX_FILE_COUNT];

Sector* init_sector() {
	Sector* fs = malloc(sizeof(Sector));
	fs->next = END_SECTOR;
	for (unsigned int i = 0; i < FS_SECTOR_DATA_SIZE; ++i)
		fs->data[i] = 0;
	return fs;
}

File* find_file(char* name) {
	for (int i = 0; i < findex_end; ++i)
		if (strcmp(findex[i]->name, name) == 0)
			return findex[i];
	return NULL;
}

int file_size(char* name) {
	File* fp = find_file(name);
	if (!fp) return 0;

	Sector* fs = fp->first_sector;
	int size = FS_SECTOR_DATA_SIZE;
	while (fs->next != END_SECTOR) {
		fs = fs->next;
		size += FS_SECTOR_DATA_SIZE;
	}
	return size;
}

int file_count() {
	return findex_end;
}

char* file_get_name(int id) {
	if (id >= 0 && id < findex_end && findex[id] != NULL)
		return findex[id]->name;
	return NULL;
}

int file_make(char* name) {
	if (findex_end >= FS_MAX_FILE_COUNT)
		return FILE_COUNT_MAX_EXCEEDED;

	File* fp = malloc(sizeof(File));
	strcpy(fp->name, name);
	fp->first_sector = init_sector();

	findex[findex_end] = fp;
	findex_end += 1;

	return OK;
}

int file_read(char* filename, char* output) {
	File* fp = find_file(filename);
	if (!fp) return FILE_NOT_FOUND;

	Sector* fs = fp->first_sector;
	do {
		for (unsigned int i = 0; i < FS_SECTOR_DATA_SIZE; ++i)
			output[i] = fs->data[i];

		output += FS_SECTOR_DATA_SIZE;
		fs = fs->next;
	} while(fs != END_SECTOR);

	return OK;
}

int file_write(char* filename, char* data, nat32 depth) {
	File* fp = find_file(filename);
	if (!fp) return FILE_NOT_FOUND;

	Sector* fs = fp->first_sector;
	char* end = data + depth;

	while (data < end) {
		size_t remaining = end - data;
		if (remaining <= FS_SECTOR_DATA_SIZE) {
			for (size_t i = 0; i < remaining; ++i)
				fs->data[i] = data[i];
			data = end;
		} else {
			for (size_t i = 0; i < FS_SECTOR_DATA_SIZE; ++i)
				fs->data[i] = data[i];
			data += FS_SECTOR_DATA_SIZE;

			fs->next = init_sector();
			fs = fs->next;
		}
	}

	return OK;
}

int file_write_str(char* filename, char* text) {
  return file_write(filename, text, strlen(text));
}

void fs_init() {
	file_make("fuck.txt");
	file_make("fuc2k.txt");
    file_write_str("fuck.txt", "file content wqow");
}
