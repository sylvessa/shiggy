#ifndef PIC_H
#define PIC_H

#include "types.h"

/* https://wiki.osdev.org/PIC#Programming_with_the_8259_PIC */

#define PIC_MASTER 0x20 // IO base address for master pic

#define PIC_MASTER_COMMAND_PORT PIC_MASTER // IO port address for master pic - command

#define PIC_MASTER_DATA_PORT (PIC_MASTER + 1) // IO port address for master pic - data

#define PIC_SLAVE 0xA0 // io base address for slave pic

#define PIC_SLAVE_COMMAND_PORT PIC_SLAVE // IO port address for slave pic - command

#define PIC_SLAVE_DATA_PORT (PIC_SLAVE + 1) // IO port address for slave pic - data

#define PIC_INIT_COMMAND 0x10 // the initialise command for two pcis
#define PIC_INIT_COMMAND_ICW4_NEEDED_PARAM 0x01
#define PIC_INITIALIZATION_COMMAND (PIC_INIT_COMMAND | PIC_INIT_COMMAND_ICW4_NEEDED_PARAM)

#define PIC_MASTER_OFFSET 0x20
#define PIC_SLAVE_OFFSET 0x28

#define PIC_8086_MODE 0x01 // 8086/88 (MCS-80/85) mode (lol)

void pci_init();

#endif