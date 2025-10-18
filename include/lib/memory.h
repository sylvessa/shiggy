#ifndef MEMORY_H
#define MEMORY_H

#include "types.h"

typedef struct block {
	word size;
	struct block *next;
} block_t;

void memcpy(byte *source, byte *dest, nat32 n);
int32 memcmp(const byte *ptr1, const byte *ptr2, nat32 size);
void *memmove(byte *dest, const byte *src, nat32 n);
void *malloc(nat32 nBytes);
void *calloc(nat32 n, nat32 size);
void free(void *ptr);

#define low_16(address) (nat16)((address)&0xFFFF)
#define high_16(address) (nat16)(((address) >> 16) & 0xFFFF)
#define low_8(address) (nat16)((address)&0xFF)
#define high_8(address) (nat16)(((address) >> 8) & 0xFF)

#endif