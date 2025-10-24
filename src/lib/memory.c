#include "globals.h"
#include "lib/memory.h"

static block_t *free_list = 0;
static word free_mem_addr = (word)&__free_memory_start;

void *memcpy(void *d, const void *s, nat32 n) {
	byte *a = d;
	const byte *b = s;
	while (n--) *a++ = *b++;
	return d;
}

void *memmove(void *d, const void *s, nat32 n) {
	byte *a = d;
	const byte *b = s;
	if (a < b) while (n--) *a++ = *b++;
	else for (nat32 i = n; i--; ) a[i] = b[i];
	return d;
}

void *memset(void *p, int v, nat32 n) {
	byte *b = p;
	while (n--) *b++ = (byte)v;
	return p;
}

int32 memcmp(const void *a, const void *b, nat32 n) {
	const byte *x = a, *y = b;
	while (n--) {
		if (*x != *y) return *x - *y;
		x++; y++;
	}
	return 0;
}

void *malloc(nat32 n) {
	printf("allocating at %p", free_mem_addr);
	n = (n + WORD_SIZE - 1) & ~(WORD_SIZE - 1);
	block_t *p = free_list, *q = 0;

	while (p) {
		if (p->size >= n) {
			if (q) q->next = p->next;
			else free_list = p->next;
			return p + 1;
		}
		q = p;
		p = p->next;
	}

	block_t *b = (block_t*)free_mem_addr;
	b->size = n;
	free_mem_addr += sizeof(block_t) + n;
	return b + 1;
}

void free(void *p) {
	if (!p) return;
	block_t *b = (block_t*)p - 1;
	b->next = free_list;
	free_list = b;
}

void *calloc(nat32 n, nat32 s) {
	nat32 size = n * s;
	void *p = malloc(size);
	memset(p, 0, size);
	return p;
}
