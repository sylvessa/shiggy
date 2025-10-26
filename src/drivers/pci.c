#include "globals.h"

nat32 pci_config_read_word(nat8 bus, nat8 slot, nat8 func, nat8 offset) {
	nat32 address;
	nat32 lbus = (nat32)bus;
	nat32 lslot = (nat32)slot;
	nat32 lfunc = (nat32)func;
	nat32 tmp = 0;

	address = (nat32)((lbus << 16) | (lslot << 11) | (lfunc << 8) | (offset & 0xfc) | ((nat32)0x80000000));

	out_long(PCI_CONFIG_ADDRESS, address);

	tmp = in_long(PCI_CONFIG_DATA);

	return tmp;
}

void show_pci_devices() {
	for (nat8 device = 0; device < 32; device++) {
		for (nat8 func = 0; func < 8; func++) {
			nat32 data = pci_config_read_word(0, device, func, 0);
			nat16 vendor_id = (nat16)(data & 0xFFFF);
			nat16 device_id = (nat16)(data >> 16);

			if (vendor_id != 0xFFFF) {
				printf("pci device found at %d, %d: vendor id = 0x%p, device id = 0x%p\n", device, func, vendor_id, device_id);
			}
		}
	}
}
