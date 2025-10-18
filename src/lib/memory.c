#include "globals.h"
#include "lib/memory.h"

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

void *malloc(nat32 nBytes)
{
    static word free_mem_addr = 0x10000;
    void *const ret = (void *)free_mem_addr;
    free_mem_addr += nBytes;
    if (free_mem_addr & WORD_MASK)
    {
        free_mem_addr &= ~WORD_MASK;
        free_mem_addr += WORD_SIZE;
    }
    return ret;
}

void *calloc(nat32 n, nat32 size) {
    void *ptr = malloc(n * size);
    byte *b = (byte *)ptr;
    for (nat32 i = 0; i < n * size; i++) b[i] = 0;
    return ptr;
}

