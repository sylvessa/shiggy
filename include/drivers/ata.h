#ifndef ATA_H
#define ATA_H

int ata_identify();
void init_ata();
nat32 ata_get_drive_size();
#endif