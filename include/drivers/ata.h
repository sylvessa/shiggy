#ifndef ATA_H
#define ATA_H

#include "globals.h"

int ata_identify();
void init_ata();
nat32 ata_get_drive_size();
void ata_read_sector(nat32 lba, nat8* buffer);
void ata_write_sector(nat32 lba, const nat8* buffer);

#endif
