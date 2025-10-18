#ifndef FS_H
#define FS_H

#include "fs/props.h"
#include <stdlib.h>

#define OK 0
#define FILE_COUNT_MAX_EXCEEDED 1
#define FILE_ALREADY_EXISTS 2
#define FILE_NAME_INVALID 3
#define FILE_NOT_FOUND 1
#define END_SECTOR NULL
#define FS_SECTOR_DATA_SIZE (FS_SECTOR_SIZE - sizeof(struct SectorStruct*))

typedef unsigned char byte;

struct SectorStruct
{
	struct SectorStruct* next;
	byte data[FS_SECTOR_DATA_SIZE];
};
typedef struct SectorStruct Sector;

typedef struct
{
	char name[FS_FILE_NAME_BUFFER];
	char tags[FS_FILE_TAGS_BUFFER];
	Sector* first_sector;
} File;

void fs_init();
char* file_get_name(int id);
int file_count();
int file_make(char* name);
int file_size(char* name);
int file_read(char* filename, char* output);

#endif
