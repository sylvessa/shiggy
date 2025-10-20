#include "globals.h"
#include "fs/fat32.h"
#include "lib/pio.h"
#include "drivers/screen.h"
#include "cpu/isr.h"


static nat16 ata_read_data16() {
	return in_b16(ATA_PRIMARY_IO);
}

static void ata_wait_bsy() {
	while (in_byte(ATA_PRIMARY_IO + 7) & 0x80) {}
}

static void ata_wait_drq() {
	while (!(in_byte(ATA_PRIMARY_IO + 7) & 0x08)) {}
}

void fat32_read_sector(nat32 lba, nat8 *buffer) {
	out_byte(ATA_PRIMARY_IO + 6, 0xE0 | ((lba >> 24) & 0x0F)); // select master + LBA28
	out_byte(ATA_PRIMARY_IO + 2, 1); // sector count
	out_byte(ATA_PRIMARY_IO + 3, lba & 0xFF);
	out_byte(ATA_PRIMARY_IO + 4, (lba >> 8) & 0xFF);
	out_byte(ATA_PRIMARY_IO + 5, (lba >> 16) & 0xFF);
	out_byte(ATA_PRIMARY_IO + 7, 0x20); // READ SECTORS

	ata_wait_bsy();
	ata_wait_drq();

	for (int i = 0; i < 256; i++) {
		nat16 word = ata_read_data16();
		buffer[i*2] = word & 0xFF;
		buffer[i*2+1] = word >> 8;
	}
}

void fat32_write_sector(nat32 lba, const nat8 *buffer) {
	out_byte(ATA_PRIMARY_IO + 6, 0xE0 | ((lba >> 24) & 0x0F));
	out_byte(ATA_PRIMARY_IO + 2, 1);
	out_byte(ATA_PRIMARY_IO + 3, lba & 0xFF);
	out_byte(ATA_PRIMARY_IO + 4, (lba >> 8) & 0xFF);
	out_byte(ATA_PRIMARY_IO + 5, (lba >> 16) & 0xFF);
	out_byte(ATA_PRIMARY_IO + 7, 0x30); // WRITE SECTORS

	ata_wait_bsy();
	ata_wait_drq();

	for (int i = 0; i < 256; i++) {
		nat16 word = buffer[i*2] | (buffer[i*2+1] << 8);
		__asm__ __volatile__("out %%ax, %%dx" :: "a"(word), "d"(ATA_PRIMARY_IO));
	}

	// flush cache
	out_byte(ATA_PRIMARY_IO + 7, 0xE7);
}

int fat32_is_formatted() {
	nat8 buffer[SECTOR_SIZE];
	fat32_read_sector(0, buffer);

	if (buffer[82] == 'F' && buffer[83] == 'A' &&
	    buffer[84] == 'T' && buffer[85] == '3' &&
	    buffer[86] == '2') {
		//print("drive appears formatted (FAT32)\n");
		return 1;
	} else {
		//print("drive not formatted\n");
		return 0;
	}
}

void fat32_format() {
	nat8 buffer[SECTOR_SIZE] = {0};

	buffer[0] = 0xEB;
	buffer[1] = 0x58;
	buffer[2] = 0x90;

	buffer[3] = 'S';
	buffer[4] = 'H';
	buffer[5] = 'I';
	buffer[6] = 'G';
	buffer[7] = 'G';
	buffer[8] = 'Y';
	buffer[9] = ' ';
	buffer[10] = ' ';

	// bytes per sector: 512
	buffer[11] = 0x00;
	buffer[12] = 0x02;

	// sectors per cluster: 1
	buffer[13] = 0x01;

	// reserved sectors: 32
	buffer[14] = 32;
	buffer[15] = 0x00;

	// number of FATs: 2
	buffer[16] = 2;

	// max root entries (FAT32 ignores)
	buffer[17] = 0;
	buffer[18] = 0;

	// total sectors (small disk, will use LBA32 later)
	buffer[19] = 0;
	buffer[20] = 0;

	// media descriptor
	buffer[21] = 0xF8;

	// sectors per FAT (FAT12/16 ignored)
	buffer[22] = 0;
	buffer[23] = 0;

	// FAT32 specific fields
	nat32 sectors_per_fat = 64; // arbitrary small FAT table
	buffer[36] = sectors_per_fat & 0xFF;
	buffer[37] = (sectors_per_fat >> 8) & 0xFF;
	buffer[38] = (sectors_per_fat >> 16) & 0xFF;
	buffer[39] = (sectors_per_fat >> 24) & 0xFF;

	// root cluster = 2
	buffer[44] = 2;
	buffer[45] = 0;
	buffer[46] = 0;
	buffer[47] = 0;

	buffer[82] = 'F';
	buffer[83] = 'A';
	buffer[84] = 'T';
	buffer[85] = '3';
	buffer[86] = '2';
	buffer[87] = ' ';
	buffer[88] = ' ';
	buffer[89] = ' ';

	// boot signature 0x55AA
	buffer[510] = 0x55;
	buffer[511] = 0xAA;

	fat32_write_sector(0, buffer);

	print("drive formatted\n");
}