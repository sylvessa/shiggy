#include "globals.h"

#include "lib/pio.h"
#include "drivers/screen.h"
#include "cpu/isr.h"

static nat16 ata_read_data16(void) {
	return in_b16(ATA_PRIMARY_IO);
}

static int ata_wait_bsy_timed(unsigned int timeout_loops) {
	while (timeout_loops--) {
		nat8 st = in_byte(ATA_PRIMARY_IO + 7);
		if (!(st & ATA_STATUS_BSY)) return 0;
		io_wait();
	}
	return -1;
}

static int ata_wait_drq_timed(unsigned int timeout_loops) {
	while (timeout_loops--) {
		nat8 st = in_byte(ATA_PRIMARY_IO + 7);
		if (st & 0x01) return -2;
		if (st & ATA_STATUS_DRQ) return 0;
		io_wait();
	}
	return -1;
}

int fat32_read_sector(nat32 lba, nat8 *buffer) {
	out_byte(ATA_PRIMARY_IO + 6, 0xE0 | ((lba >> 24) & 0x0F));
	out_byte(ATA_PRIMARY_IO + 2, 1);
	out_byte(ATA_PRIMARY_IO + 3, lba & 0xFF);
	out_byte(ATA_PRIMARY_IO + 4, (lba >> 8) & 0xFF);
	out_byte(ATA_PRIMARY_IO + 5, (lba >> 16) & 0xFF);
	out_byte(ATA_PRIMARY_IO + 7, ATA_CMD_READ);

	if (ata_wait_bsy_timed(100000) != 0) {
		print("read: timeout waiting BSY clear\n");
		return -1;
	}
	if (ata_wait_drq_timed(100000) != 0) {
		print("read: timeout waiting DRQ or device error\n");
		return -2;
	}

	for (int i = 0; i < 256; i++) {
		nat16 word = ata_read_data16();
		buffer[i*2] = word & 0xFF;
		buffer[i*2+1] = (word >> 8) & 0xFF;
	}

	return 0;
}

int fat32_write_sector(nat32 lba, const nat8 *buffer) {
	out_byte(ATA_PRIMARY_IO + 6, 0xE0 | ((lba >> 24) & 0x0F));
	out_byte(ATA_PRIMARY_IO + 2, 1); /* sector count */
	out_byte(ATA_PRIMARY_IO + 3, lba & 0xFF);
	out_byte(ATA_PRIMARY_IO + 4, (lba >> 8) & 0xFF);
	out_byte(ATA_PRIMARY_IO + 5, (lba >> 16) & 0xFF);

	out_byte(ATA_PRIMARY_IO + 7, ATA_CMD_WRITE);

	io_wait();

	if (ata_wait_bsy_timed(100000) != 0) {
		print("write: timeout waiting BSY clear after cmd\n");
		return -1;
	}
	if (ata_wait_drq_timed(100000) != 0) {
		print("write: timeout waiting DRQ before data or device error\n");
		return -2;
	}

	for (int i = 0; i < 256; i++) {
		nat16 word = buffer[i*2] | (buffer[i*2+1] << 8);
		__asm__ __volatile__("out %%ax, %%dx" :: "a"(word), "d"(ATA_PRIMARY_IO));
		if ((i & 0x3F) == 0x3F) io_wait();
	}

	out_byte(ATA_PRIMARY_IO + 7, ATA_CMD_FLUSH);

	if (ata_wait_bsy_timed(1000000) != 0) {
		print("write: timeout waiting BSY clear after flush\n");
		return -3;
	}

	nat8 s = in_byte(ATA_PRIMARY_IO + 7);
	if (s & 0x01) { // Oops
		print("write: device reported error after write\n");
		return -4;
	}

	return 0;
}

int fat32_is_formatted(void) {
	nat8 buffer[SECTOR_SIZE];
	if (fat32_read_sector(0, buffer) != 0) return 0;

	if (buffer[82] == 'F' && buffer[83] == 'A' &&
	    buffer[84] == 'T' && buffer[85] == '3' &&
	    buffer[86] == '2') {
		return 1;
	}
	return 0;
}

void fat32_format(void) {
	nat8 buffer[SECTOR_SIZE];
	for (int i=0;i<SECTOR_SIZE;i++) buffer[i]=0;

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

	buffer[11] = 0x00; buffer[12] = 0x02; // bytes/sector = 512
	buffer[13] = 0x01; // sectors/cluster = 1
	buffer[14] = 32; buffer[15] = 0x00; // reserved sectors = 32
	buffer[16] = 2; // number of FATs

	// sectors per FAT (32-bit value at offset 36)
	nat32 sectors_per_fat = 64;
	buffer[36] = sectors_per_fat & 0xFF;
	buffer[37] = (sectors_per_fat >> 8) & 0xFF;
	buffer[38] = (sectors_per_fat >> 16) & 0xFF;
	buffer[39] = (sectors_per_fat >> 24) & 0xFF;

	buffer[44] = 2; buffer[45]=0; buffer[46]=0; buffer[47]=0;

	buffer[82] = 'F'; 
	buffer[83] = 'A'; 
	buffer[84] = 'T';
	buffer[85] = '3'; 
	buffer[86] = '2'; 
	buffer[87] = ' '; 
	buffer[88] = ' '; 
	buffer[89] = ' ';

	buffer[510] = 0x55; buffer[511] = 0xAA;

	if (fat32_write_sector(0, buffer) != 0) {
		print("fat32_format: failed to write boot sector\n");
	}
}
