// testing fat32
#include <stdio.h>
#include <stdarg.h>
#include "fs/fat32.h"

static FILE* disk = NULL;

nat8 ata_read_sector(nat32 lba, nat8* buffer) {
	if (!disk)
		return 0;
	if (fseek(disk, (long)lba * 512, SEEK_SET) != 0)
		return 0;
	return fread(buffer, 1, 512, disk) == 512;
}

void ata_write_sector(nat32 lba, const nat8* buffer) {
	if (!disk)
		return;
	fseek(disk, (long)lba * 512, SEEK_SET);
	fwrite(buffer, 1, 512, disk);
	fflush(disk);
}

int ata_identify(void) { return 1; }
nat32 ata_get_drive_size(void) { return 131072; } // 64 MB

void print(const char* s) { printf("%s", s); }

int printf(const char* fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	int r = vprintf(fmt, ap);
	va_end(ap);
	return r;
}

unsigned long strlen(const char* s) {
	unsigned long n = 0;
	while (s[n])
		n++;
	return n;
}

int strcmp(const char* a, const char* b) {
	while (*a && *a == *b) {
		a++;
		b++;
	}
	return (int)((unsigned char)*a - (unsigned char)*b);
}

int strcasecmp(const char* a, const char* b) {
	while (*a && *b) {
		char ca = *a, cb = *b;
		if (ca >= 'a' && ca <= 'z')
			ca -= 32;
		if (cb >= 'a' && cb <= 'z')
			cb -= 32;
		if (ca != cb)
			return (int)((unsigned char)ca - (unsigned char)cb);
		a++;
		b++;
	}
	return (int)((unsigned char)*a - (unsigned char)*b);
}

void strcpy(char* d, const char* s) {
	while ((*d++ = *s++))
		;
}

void strncpy(char* d, const char* s, unsigned long n) {
	unsigned long i = 0;
	for (; i < n && s[i]; i++)
		d[i] = s[i];
	if (i < n)
		d[i] = 0;
}

void strcat(char* d, const char* s) {
	while (*d)
		d++;
	while ((*d++ = *s++))
		;
}

char* strrchr(const char* s, int c) {
	const char* last = NULL;
	while (*s) {
		if (*s == (char)c)
			last = s;
		s++;
	}
	return (char*)last;
}

int current_dir_cluster = 2;
char* current_dir = "/";

static const char* long_content = "the quick brown fox jumps over the lazy dog. "
								  "pack my box with five dozen liquor jugs. "
								  "how vexingly quick daft zebras jump! "
								  "sphinx of black quartz, judge my vow. "
								  "the five boxing wizards jump quickly. "
								  "jived fox nymph grabs quick waltz. "
								  "glib jocks quiz nymph to vex dwarf. "
								  "brick quiz whangs jumpy veldt fox. "
								  "bright vixens jump; dozy fowl quack. "
								  "quick zephyrs blow, vexing daft jim. "
								  "wah wah wah wah, real fat32 now :3   ";

int main(int argc, char** argv) {
	const char* path = argc > 1 ? argv[1] : "test.img";

	disk = fopen(path, "w+b");
	if (!disk) {
		printf("cannot open %s\n", path);
		return 1;
	}

	// zero fill 64 MB
	{
		char zero[65536];
		for (unsigned i = 0; i < sizeof(zero); i++)
			zero[i] = 0;
		for (int i = 0; i < 64 * 1024 * 1024 / (int)sizeof(zero); i++)
			fwrite(zero, 1, sizeof(zero), disk);
		fflush(disk);
	}

	printf("== format ==\n");
	fat32_fs_init();
	printf("is_formatted: %d\n", is_formatted());

	printf("\n== create files ==\n");
	printf("mk hello.txt: %d\n", fat32_create_file(FAT32_ROOT_CLUSTER, "hello.txt", "hello world from shiggy fat32"));
	printf("mk simple (no ext): %d\n", fat32_create_file(FAT32_ROOT_CLUSTER, "simple", "no extension"));
	printf("mk long name + long content (2+ clusters): %d\n",
		   fat32_create_file(FAT32_ROOT_CLUSTER, "loremipsumdolorsit.txt", long_content));
	printf("mk lowercase long name: %d\n",
		   fat32_create_file(FAT32_ROOT_CLUSTER, "this is a fairly long filename.txt", "longfilename"));

	printf("\n== create dirs ==\n");
	printf("mkdir subdir: %d\n", fat32_create_dir(FAT32_ROOT_CLUSTER, "subdir"));

	{
		printf("\n== files in subdir ==\n");
		nat32 total = fat32_file_count(FAT32_ROOT_CLUSTER) + fat32_dir_count(FAT32_ROOT_CLUSTER);
		printf("root entries: %d (files %d, dirs %d)\n", (int)total, (int)fat32_file_count(FAT32_ROOT_CLUSTER),
			   (int)fat32_dir_count(FAT32_ROOT_CLUSTER));
		for (nat32 i = 0; i < total; i++) {
			fat32_entry_info_t info;
			if (!fat32_dir_get_entry(FAT32_ROOT_CLUSTER, i, &info))
				break;
			printf("  [%d] %s %s (%d bytes, cluster %d)\n", (int)i, info.name,
				   (info.attr & FAT32_ATTR_DIRECTORY) ? "DIR" : "FILE", (int)info.file_size, (int)info.first_cluster);
		}
	}

	{
		printf("\n== create + read back in subdir ==\n");
		nat32 total = fat32_file_count(FAT32_ROOT_CLUSTER) + fat32_dir_count(FAT32_ROOT_CLUSTER);
		nat32 sub = 0;
		for (nat32 i = 0; i < total; i++) {
			fat32_entry_info_t info;
			if (!fat32_dir_get_entry(FAT32_ROOT_CLUSTER, i, &info))
				break;
			if ((info.attr & FAT32_ATTR_DIRECTORY) && strcmp(info.name, "subdir") == 0)
				sub = info.first_cluster;
		}
		printf("subdir cluster: %d\n", (int)sub);
		printf("mkdir subfile.txt: %d\n", fat32_create_file(sub, "subfile.txt", "i am inside a directory"));
		printf("rm subfile.txt: %d\n", fat32_delete_file(sub, "subfile.txt"));
		printf("counts after rm (files %d, dirs %d)\n", (int)fat32_file_count(sub), (int)fat32_dir_count(sub));
	}

	printf("\n== read back ==\n");
	{
		nat32 sz = fat32_file_size(FAT32_ROOT_CLUSTER, "hello.txt");
		char buf[2048];
		if (fat32_read_file(FAT32_ROOT_CLUSTER, "hello.txt", buf, sizeof(buf))) {
			printf("hello.txt (%d bytes): \"%s\"\n", (int)sz, buf);
		} else {
			printf("failed to read hello.txt\n");
		}
	}

	{
		nat32 sz = fat32_file_size(FAT32_ROOT_CLUSTER, "loremipsumdolorsit.txt");
		char buf[4096];
		if (fat32_read_file(FAT32_ROOT_CLUSTER, "loremipsumdolorsit.txt", buf, sizeof(buf))) {
			printf("lorem...txt (%d bytes) readback match: %d\n", (int)sz,
				   (int)(strlen(buf) == strlen(long_content) && strcmp(buf, long_content) == 0));
		} else {
			printf("failed to read lorem...txt\n");
		}
	}

	{
		printf("size of longfilename: %d\n",
			   (int)fat32_file_size(FAT32_ROOT_CLUSTER, "this is a fairly long filename.txt"));
	}

	printf("\n== delete ==\n");
	printf("rm simple: %d\n", fat32_delete_file(FAT32_ROOT_CLUSTER, "simple"));
	printf("rm missing file: %d (expect 0)\n", fat32_delete_file(FAT32_ROOT_CLUSTER, "nope.txt"));

	printf("\n== parent nav ==\n");
	{
		nat32 total = fat32_file_count(FAT32_ROOT_CLUSTER) + fat32_dir_count(FAT32_ROOT_CLUSTER);
		for (nat32 i = 0; i < total; i++) {
			fat32_entry_info_t info;
			if (!fat32_dir_get_entry(FAT32_ROOT_CLUSTER, i, &info))
				break;
			if (info.attr & FAT32_ATTR_DIRECTORY) {
				nat32 parent = 0;
				fat32_dir_parent(info.first_cluster, &parent);
				printf("parent of %s is cluster %d (root=%d)\n", info.name, (int)parent, FAT32_ROOT_CLUSTER);
			}
		}
	}

	fclose(disk);
	printf("\n== done ==\n");
	return 0;
}