#include "globals.h"
#include "lib/memory.h"

static block_t *free_list = 0;
static word free_mem_addr = 0x50000;

void *memcpy(void *dest, const void *source, nat32 no_bytes) {
	byte *d = (byte*)dest;
	const byte *s = (const byte*)source;
	for (nat32 i = 0; i < no_bytes; i++) d[i] = s[i];
	return dest;
}

void *memmove(void *dest, const void *src, nat32 n) {
	byte *d = (byte*)dest;
	const byte *s = (const byte*)src;
	if (d < s) {
		for (nat32 i = 0; i < n; i++) d[i] = s[i];
	} else if (d > s) {
		for (nat32 i = n; i > 0; i--) d[i-1] = s[i-1];
	}
	return dest;
}

void *memset(void *ptr, int value, nat32 n) {
	byte *b = (byte*)ptr;
	for (nat32 i = 0; i < n; i++) b[i] = (byte)value;
	return ptr;
}


int32 memcmp(const void *ptr1, const void *ptr2, nat32 size) {
	const byte *p1 = (const byte*)ptr1;
	const byte *p2 = (const byte*)ptr2;
	while (size-- > 0) {
		if (*p1 != *p2) return *p1 - *p2;
		++p1; ++p2;
	}
	return 0;
}

void *malloc(nat32 nBytes) {
	nBytes = (nBytes + WORD_SIZE - 1) & ~(WORD_SIZE - 1);
	block_t *prev = 0, *curr = free_list;

	while (curr) {
		if ((nat32)curr->size >= nBytes) {
			if (prev) prev->next = curr->next;
			else free_list = curr->next;
			return (void*)(curr + 1);
		}
		prev = curr; curr = curr->next;
	}

	block_t *b = (block_t*)free_mem_addr;
	b->size = nBytes;
	free_mem_addr += nBytes + sizeof(block_t);
	return (void*)(b + 1);
}

void free(void *ptr) {
	if (!ptr) return;
	block_t *b = (block_t*)ptr - 1;
	b->next = free_list;
	free_list = b;
}

void *calloc(nat32 n, nat32 size) {
	void *ptr = malloc(n * size);
	byte *b = (byte*)ptr;
	for (nat32 i = 0; i < n * size; i++) b[i] = 0;
	return ptr;
}
