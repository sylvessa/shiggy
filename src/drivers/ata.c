#include "globals.h"
#include "cpu/isr.h"
#include "drivers/screen.h"
#include "drivers/ata.h"

#define ATA_PRIMARY_IO 0x1F0
#define ATA_PRIMARY_CTRL 0x3F6

static int ata_irq_triggered = 0;

void ata_irq_handler(registers_t r) {
	// acknowledge pic
	out_byte(0x20, 0x20);
	// read status to clear interrupt
	in_byte(ATA_PRIMARY_IO + 7);
	ata_irq_triggered = 1;
}

void init_ata() {
	register_interrupt_handler(IRQ_BASE + 14, ata_irq_handler);
}

void ata_print_size(nat16 *buffer) {
	nat32 sectors = ((nat32)buffer[61] << 16) | buffer[60];
	nat32 mb = sectors / 2048; // 512 * 2048 = 1 MB
	printf("drive size: %d MB\n", mb);
}

static nat16 ata_read_data16() {
	nat16 data = in_b16(ATA_PRIMARY_IO);
	return data;
}

void ata_identify() {
	out_byte(ATA_PRIMARY_CTRL, 0x0); // enable interrupts
	out_byte(ATA_PRIMARY_IO + 6, 0xA0); // select master drive
	out_byte(ATA_PRIMARY_IO + 2, 0x0); // sector count
	out_byte(ATA_PRIMARY_IO + 3, 0x0);
	out_byte(ATA_PRIMARY_IO + 4, 0x0);
	out_byte(ATA_PRIMARY_IO + 5, 0x0);
	out_byte(ATA_PRIMARY_IO + 7, 0xEC); // IDENTIFY command

	nat8 status = in_byte(ATA_PRIMARY_IO + 7);
	if (status == 0) {
		print("no drive present on primary master\n");
		return;
	}

	while ((status & 0x80) != 0) // wait for BSY clear
		status = in_byte(ATA_PRIMARY_IO + 7);

	nat8 cl = in_byte(ATA_PRIMARY_IO + 4);
	nat8 ch = in_byte(ATA_PRIMARY_IO + 5);
	if (cl != 0 || ch != 0) {
		print("not an ata device\n");
		return;
	}

	while (1) {
		status = in_byte(ATA_PRIMARY_IO + 7);
		if (status & 0x08) break; // DRQ set
	}

	nat16 buffer[256];
	for (int i = 0; i < 256; i++)
		buffer[i] = ata_read_data16();

	print("IDE drive detected\n");
	ata_print_size(buffer);
}