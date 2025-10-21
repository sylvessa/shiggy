#ifndef FAT32_H
#define FAT32_H

#include "globals.h"

#define ATA_PRIMARY_IO 0x1F0
#define ATA_PRIMARY_CTRL 0x3F6
#define ATA_STATUS_BSY 0x80
#define ATA_STATUS_DRQ 0x08
#define ATA_CMD_READ  0x20
#define ATA_CMD_WRITE 0x30
#define ATA_CMD_FLUSH 0xE7

#define FAT32_ROOT_CLUSTER 2
#define SECTOR_SIZE 512
#define FAT32_SECTORS_PER_CLUSTER 1
#define FAT32_CLUSTER_SIZE (SECTOR_SIZE * FAT32_SECTORS_PER_CLUSTER)

// lba of first data sector: reserved sectors + FATs
#define FAT32_RESERVED_SECTORS 32
#define FAT32_FAT_COUNT 2
#define FAT32_SECTORS_PER_FAT 64
#define FAT32_DATA_START (FAT32_RESERVED_SECTORS + FAT32_FAT_COUNT * FAT32_SECTORS_PER_FAT)

// imported from old fs/main.h
#include <stdlib.h>

#define FS_SECTOR_SIZE 512
#define FS_MAX_FILE_COUNT 1000
#define FS_FILE_NAME_BUFFER 512
#define FS_FILE_TAGS_BUFFER 512
#define FS_FILE_NAME_VALID_CHARS "qwertyuiiopasdfghjklzxcvbnm1234567890QWERTYUIOPASDFGHJKLZXCVBNM"
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

// continue


int fat32_is_formatted();
void fat32_format();
int fat32_read_sector(nat32 lba, nat8 *buffer);
int fat32_write_sector(nat32 lba, const nat8 *buffer);

#endif
