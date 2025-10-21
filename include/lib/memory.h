#ifndef MEMORY_H
#define MEMORY_H

#include "types.h"

typedef struct block {
	word size;
	struct block *next;
} block_t;

void *memcpy(void *dest, const void *source, nat32 no_bytes);
void *memmove(void *dest, const void *src, nat32 n);
int32 memcmp(const void *ptr1, const void *ptr2, nat32 size);
void *memset(void *ptr, int value, nat32 n);

void *malloc(nat32 nBytes);
void free(void *ptr);
void *calloc(nat32 n, nat32 size);

#define low_16(address) (nat16)((address)&0xFFFF)
#define high_16(address) (nat16)(((address) >> 16) & 0xFFFF)
#define low_8(address) (nat16)((address)&0xFF)
#define high_8(address) (nat16)(((address) >> 8) & 0xFF)

#endif