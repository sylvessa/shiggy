#include "globals.h"
#include "apps/base.h"
#include "drivers/cmos.h"

void cmd_test(const char* args) {
	print("Normal text \\4FRed on white\\x back to white on black\n");
    print("\\2 Fuck\n");
    print("\\e1Yellow on blue\n");
}
