#ifndef NETWORK_MAIN_H
#define NETWORK_MAIN_H

#include "types.h"

#define RTL8139_VENDOR_ID 0x10EC
#define RTL8139_DEVICE_ID 0x8139
#define RTL8139_REG_COMMAND 0x37
#define RTL8139_CMD_RESET 0x10
#define RTL8139_REG_INTR_STATUS 0x3E
#define RTL8139_CMD_RX_EN 0x08
#define RTL8139_CMD_TX_EN 0x04

#define RTL8139_DETECT_TIMEOUT 1000000

typedef struct
{
	nat32 txBuffer;
	nat32 rxBuffer;
	byte mac[6];
} rtl8139;

extern rtl8139 rtl8139Device;

void init_network();
byte* get_mac();
void send_frame(byte* data, int len);

#endif