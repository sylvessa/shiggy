#ifndef PIO_H
#define PIO_H
#include "types.h"

byte in_byte(nat16 port);
void out_byte(nat16 port, byte data);
b16 in_b16(nat16 port);
void out_b16(nat16 port, b16 data);
void io_wait();

#endif