#ifndef PCI_H
#define PCI_H

#include "types.h"

#define PCI_CONFIG_ADDRESS 0xCF8
#define PCI_CONFIG_DATA 0xCFC

nat32 pci_config_read_word(nat8 bus, nat8 slot, nat8 func, nat8 offset);
void show_pci_devices();

#endif