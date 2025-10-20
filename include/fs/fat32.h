#ifndef FAT32_H
#define FAT32_H

#include "globals.h"

#define SECTOR_SIZE 512
#define ATA_PRIMARY_IO 0x1F0
#define ATA_PRIMARY_CTRL 0x3F6

int fat32_is_formatted();
void fat32_format();
void fat32_read_sector(nat32 lba, nat8 *buffer);
void fat32_write_sector(nat32 lba, const nat8 *buffer);

#endif
