#include "globals.h"
#include "lib/memory.h"

static block_t *free_list = 0;
static word free_mem_addr = 0x10000;

void memcpy(byte *source, byte *dest, nat32 no_bytes) {
	for (nat32 i = 0; i < no_bytes; i++)
		*(dest + i) = *(source + i);
}

int32 memcmp(const byte *ptr1, const byte *ptr2, nat32 size) {
	while (size-- > 0) {
		if (*ptr1 != *ptr2)
			return *ptr1 - *ptr2;
		++ptr1;
		++ptr2;
	}
	return 0;
}

void *memmove(byte *dest, const byte *src, nat32 n) {
	if (dest < src) {
		for (nat32 i = 0; i < n; i++)
			dest[i] = src[i];
	} else if (dest > src) {
		for (nat32 i = n; i > 0; i--)
			dest[i-1] = src[i-1];
	}
	return dest;
}

void *malloc(nat32 nBytes) {
	nBytes = (nBytes + WORD_SIZE - 1) & ~(WORD_SIZE - 1);

	block_t *prev = 0;
	block_t *curr = free_list;

	while (curr) {
		if ((nat32)curr->size >= nBytes) {
			if (prev) prev->next = curr->next;
			else free_list = curr->next;
			return (void *)(curr + 1);
		}
		prev = curr;
		curr = curr->next;
	}

	block_t *b = (block_t *)free_mem_addr;
	b->size = nBytes;
	free_mem_addr += nBytes + sizeof(block_t);
	return (void *)(b + 1);
}

void free(void *ptr) {
	if (!ptr) return;
	block_t *b = (block_t *)ptr - 1;
	b->next = free_list;
	free_list = b;
}

void *calloc(nat32 n, nat32 size) {
	void *ptr = malloc(n * size);
	byte *b = (byte *)ptr;
	for (nat32 i = 0; i < n * size; i++) b[i] = 0;
	return ptr;
}
