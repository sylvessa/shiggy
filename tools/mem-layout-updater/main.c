#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define ELF "build/kernel.elf"
#define OUT "docs/memory-layout.md"

int main() {
	FILE *fp = popen("objdump -h " ELF, "r");
	if (!fp) { perror("popen"); return 1; }

	FILE *out = fopen(OUT, "w");
	if (!out) { perror("fopen"); return 1; }

	time_t t = time(NULL);
	struct tm tm = *localtime(&t);
	fprintf(out, "## updated %02d/%02d/%04d\n\n", tm.tm_mon+1, tm.tm_mday, tm.tm_year+1900);

	fprintf(out, "| address / range   |                                               |\n");
	fprintf(out, "| ----------------- | --------------------------------------------- |\n");
	fprintf(out, "| 0x7C00            | boot sector loaded by BIOS                    |\n");
	fprintf(out, "| 0x7C00 - 0x7DFF   | boot sector code                              |\n");
	fprintf(out, "| 0x7E00 - 0x0FFFF  | bootloader stack / temporary buffers          |\n");

	char line[512];
	const char *sections[] = {".text", ".rodata", ".data", ".bss"};
	const char *labels[] = {
		"kernel `.text` section (code)",
		"kernel `.rodata` section (read-only data)",
		"kernel `.data` section (initialized globals)",
		"kernel `.bss` section (zero-initialized data)"
	};

	while (fgets(line, sizeof(line), fp)) {
		for (int i = 0; i < 4; i++) {
			if (strstr(line, sections[i])) {
				unsigned long size = 0, vma = 0;
				// sscanf: index, name, size, vma, lma, file offset, align
				if (sscanf(line, " %*d %*s %lx %lx", &size, &vma) == 2) {
					unsigned long end = vma + size - 1;
					fprintf(out, "| 0x%lX - 0x%lX | %s |\n", vma, end, labels[i]);
				} else {
					printf("DEBUG: failed to parse line for section %s\n", sections[i]);
				}
			}
		}
	}

	fclose(out);
	pclose(fp);
	printf("updated %s\n", OUT);
	return 0;
}
