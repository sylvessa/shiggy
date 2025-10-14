#include "globals.h"
#include "lib/ext2.h"

void cmd_ls(const char* args) {
    (void)args;
    if (ext2_list_root() != 0) {
        print("ls: failed to list root or no fs\n");
    }
}
