#include "network/main.h"
#include "globals.h"

// just tests i dont think i will take this anywhere yet

static nat32 iobase;
static nat32 rx_buffer;

static nat8 RTL8139SLOT;
static nat8 RTL8139BUS;

rtl8139 rtl8139Device;

byte* get_mac() {
	rtl8139Device.mac[0] = in_byte(iobase + 0x00);
	rtl8139Device.mac[1] = in_byte(iobase + 0x01);
	rtl8139Device.mac[2] = in_byte(iobase + 0x02);
	rtl8139Device.mac[3] = in_byte(iobase + 0x03);
	rtl8139Device.mac[4] = in_byte(iobase + 0x04);
	rtl8139Device.mac[5] = in_byte(iobase + 0x05);

	return rtl8139Device.mac;
}

nat32 RTL8139_FIND_DEVICE() {
	nat32 timeout_counter = 0;

	for (nat16 bus = 0; bus < 256; bus++) {
		for (nat8 slot = 0; slot < 32; slot++) {
			if (++timeout_counter > RTL8139_DETECT_TIMEOUT) {
				return 0x00;
			}

			nat32 vendorDevice = pci_config_read_word(bus, slot, 0, 0x00);

			if (vendorDevice == (nat32)((RTL8139_DEVICE_ID << 16) | RTL8139_VENDOR_ID)) {
				nat32 bar = pci_config_read_word(bus, slot, 0, 0x10);

				if (bar & 0x01) {
					RTL8139BUS = bus;
					RTL8139SLOT = slot;

					return bar & ~0x3;
				}
			}
		}
	}

	return 0x00;
}

void init_rtl8139() {
	out_byte(iobase + 0x52, 0x00);
	out_byte(iobase + RTL8139_REG_COMMAND, RTL8139_CMD_RESET); // software reset

	while ((in_byte(iobase + RTL8139_REG_COMMAND) & RTL8139_CMD_RESET) != 0x00) {
	}

	get_mac();

	// init recieve buf
	rx_buffer = (nat32)malloc(8192 + 16);

	if (!rx_buffer) {
		print("failed to alloc memory Z??\n");
		return;
	}

	// interrupts
	out_long(iobase + RTL8139_REG_INTR_STATUS, 0xFFFF);
	out_b16(iobase + RTL8139_REG_COMMAND, 0x0C);

	// rx&tx
	out_byte(iobase + RTL8139_REG_COMMAND, RTL8139_CMD_RX_EN | RTL8139_CMD_TX_EN);
}

void rtl8139_handler() {
	word intr_status = in_b16(iobase + RTL8139_REG_INTR_STATUS);
	print("RTL8139 GOT something\n");

	if (intr_status & 0x01) {
		// goit a packet
		print("RTL8139 GOT PACKET\n");
	}
}

void init_network() {
	iobase = RTL8139_FIND_DEVICE();

	if (iobase == 0x00) {
		print("RTL8139 not found\n");
	} else {
		print("found RTL8139\n");
		init_rtl8139();

		byte irq = pci_config_read_word(RTL8139BUS, RTL8139SLOT, 0x00, 0x3C) & 0xFF;
		printf("got irq for RTL8139: 0x%p (%d)\n", irq, IRQ_BASE + irq);

		byte* mac = get_mac();
		printf("MAC: %p:%p:%p:%p:%p:%p\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

		register_interrupt_handler(IRQ_BASE + irq, rtl8139_handler);
	}
}