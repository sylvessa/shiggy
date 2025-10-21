#include "globals.h"
#include "apps/base.h"
#include "fs/fat32.h"

void cmd_ls(const char** args, int argc) {
    nat32 file_count = fat32_file_count();
    printf("[ls] file count: %d\n", file_count);

    for (int i = 0; i < file_count; i++) {
        printf("[ls] fetching name for index %d\n", i);
        const char* name = fat32_file_get_name(i);
        if (!name) {
            printf("[ls] got NULL name for index %d, skipping\n", i);
            continue;
        }

        nat32 size = fat32_file_size(name);
        printf("[ls] name: %s, size: %d bytes\n", name, size);

        // extra: dump first few bytes of file content to verify FAT
        char buffer[32];
        if (fat32_read_file(name, buffer, sizeof(buffer))) {
            printf("[ls] first bytes: ");
            for (int b = 0; b < 8 && b < size; b++) printf("%x ", (unsigned char)buffer[b]);
            printf("\n");
        } else {
            printf("[ls] failed to read file %s\n", name);
        }
    }

    printf("[ls] done listing files\n");
}

static struct command_reg ls_command = {
    .name = "ls",
    .description = "lists files (no args rn)",
    .hidden = false,
    .func = cmd_ls
};

__attribute__((used, section(".cmds"))) static struct command_reg* ls_ptr = &ls_command;
