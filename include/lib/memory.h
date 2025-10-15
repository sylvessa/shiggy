#ifndef MEMORY_H
#define MEMORY_H

#include "types.h"

void memcpy(byte *source, byte *dest, nat32 n);
int32 memcmp(const byte *ptr1, const byte *ptr2, nat32 size);

#define low_16(address) (nat16)((address)&0xFFFF)
#define high_16(address) (nat16)(((address) >> 16) & 0xFFFF)
#define low_8(address) (nat16)((address)&0xFF)
#define high_8(address) (nat16)(((address) >> 8) & 0xFF)

#endif