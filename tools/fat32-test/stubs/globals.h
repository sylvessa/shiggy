#ifndef GLOBALS_STUB_H
#define GLOBALS_STUB_H

typedef unsigned char nat8;
typedef unsigned short nat16;
typedef unsigned int nat32;
typedef int bool;
typedef int STATUS;
#define true 1
#define false 0
#ifndef NULL
#define NULL 0
#endif

int ata_identify(void);
nat32 ata_get_drive_size(void);
nat8 ata_read_sector(nat32 lba, nat8* buffer);
void ata_write_sector(nat32 lba, const nat8* buffer);

void print(const char* str);
int printf(const char* fmt, ...);

void* memcpy(void* dest, const void* src, unsigned long n);
void* memmove(void* dest, const void* src, unsigned long n);
int memcmp(const void* a, const void* b, unsigned long n);
void* memset(void* p, int v, unsigned long n);
void* malloc(unsigned long n);
void free(void* p);
void* calloc(unsigned long n, unsigned long sz);

unsigned long strlen(const char* s);
int strcmp(const char* a, const char* b);
int strcasecmp(const char* a, const char* b);
void strcpy(char* d, const char* s);
void strncpy(char* d, const char* s, unsigned long n);
void strcat(char* d, const char* s);
char* strrchr(const char* s, int c);

extern int current_dir_cluster;
extern char* current_dir;

#endif