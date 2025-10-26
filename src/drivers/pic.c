#include "drivers/pic.h"
#include "globals.h"

void pic_init() {
	// save masks
	byte b1, b2;
	b1 = in_byte(PIC_MASTER_DATA_PORT);
	b2 = in_byte(PIC_SLAVE_DATA_PORT);

	out_byte(PIC_MASTER_COMMAND_PORT, PIC_INITIALIZATION_COMMAND);
	io_wait();
	out_byte(PIC_SLAVE_COMMAND_PORT, PIC_INITIALIZATION_COMMAND);
	io_wait();
	out_byte(PIC_MASTER_DATA_PORT, PIC_MASTER_OFFSET);
	io_wait();
	out_byte(PIC_SLAVE_DATA_PORT, PIC_SLAVE_OFFSET);
	io_wait();
	out_byte(PIC_MASTER_DATA_PORT, 0x04);
	io_wait();
	out_byte(PIC_SLAVE_DATA_PORT, 0x02);
	io_wait();
	out_byte(PIC_MASTER_DATA_PORT, PIC_8086_MODE);
	io_wait();
	out_byte(PIC_SLAVE_DATA_PORT, PIC_8086_MODE);
	io_wait();

	// restore masks
	out_byte(PIC_MASTER_DATA_PORT, b1);
	out_byte(PIC_SLAVE_DATA_PORT, b2);
}