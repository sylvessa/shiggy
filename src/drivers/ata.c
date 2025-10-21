#include "globals.h"

#define ATA_PRIMARY_IO 0x1F0
#define ATA_PRIMARY_CTRL 0x3F6

static int ata_irq_triggered = 0;

void ata_irq_handler(registers_t r) {
	out_byte(0x20, 0x20);
	in_byte(ATA_PRIMARY_IO + 7);
	ata_irq_triggered = 1;
}

void init_ata() {
	register_interrupt_handler(IRQ_BASE + 14, ata_irq_handler);
}

static nat16 ata_read_data16() {
	return in_b16(ATA_PRIMARY_IO);
}

nat32 ata_get_drive_size() {
	nat16 buffer[256];
	for (int i = 0; i < 256; i++)
		buffer[i] = ata_read_data16();
	return ((nat32)buffer[61] << 16) | buffer[60];
}

int ata_identify() {
	out_byte(ATA_PRIMARY_CTRL, 0x0);
	out_byte(ATA_PRIMARY_IO + 6, 0xA0);
	for (int i=2;i<=5;i++) out_byte(ATA_PRIMARY_IO+i,0x0);
	out_byte(ATA_PRIMARY_IO + 7, 0xEC);

	nat8 status = in_byte(ATA_PRIMARY_IO + 7);
	if(status==0) return 0;

	while(status & 0x80) status = in_byte(ATA_PRIMARY_IO + 7);

	if(in_byte(ATA_PRIMARY_IO + 4) != 0 || in_byte(ATA_PRIMARY_IO + 5) != 0)
		return 0;

	while(!(in_byte(ATA_PRIMARY_IO + 7) & 0x08));
	return 1;
}

static void ata_wait_bsy() {
	nat8 status;
	do {
		status = in_byte(ATA_PRIMARY_IO + 7);
	} while (status & 0x80);
}

static void ata_wait_drq() {
	nat8 status;
	do {
		status = in_byte(ATA_PRIMARY_IO + 7);
	} while (!(status & 0x08));
}

void ata_read_sector(nat32 lba, nat8 *buffer) {
	out_byte(ATA_PRIMARY_IO + 6, 0xE0 | ((lba >> 24) & 0x0F)); 
	out_byte(ATA_PRIMARY_IO + 2, 1);
	out_byte(ATA_PRIMARY_IO + 3, lba & 0xFF);
	out_byte(ATA_PRIMARY_IO + 4, (lba >> 8) & 0xFF);
	out_byte(ATA_PRIMARY_IO + 5, (lba >> 16) & 0xFF);
	out_byte(ATA_PRIMARY_IO + 7, 0x20); // READ SECTORS

	ata_wait_bsy();
	ata_wait_drq();

	for (int i = 0; i < 256; i++) {
		nat16 word = in_b16(ATA_PRIMARY_IO);
		buffer[i*2] = word & 0xFF;
		buffer[i*2+1] = word >> 8;
	}
}

void ata_write_sector(nat32 lba, const nat8 *buffer) {
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
	ata_wait_bsy();
}