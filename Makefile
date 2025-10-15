ASM=nasm
CC=gcc
LD=ld
OBJCOPY=objcopy

SRC_DIR=src
BUILD_DIR=build
INC_DIR=include

C_FLAGS=-fno-pic -fno-pie -fno-exceptions -ffreestanding -m32 -Wall -Wextra -I $(INC_DIR) -std=c17 -pedantic-errors -g
NASM_FLAGS=-f elf -I $(INC_DIR)

KERNEL_MAIN=$(SRC_DIR)/kernel/main.c
KERNEL_MAIN_OBJ=$(BUILD_DIR)/kernel/main.o

OTHER_C_SOURCES=$(shell find $(SRC_DIR) -type f -name '*.c' ! -path "$(KERNEL_MAIN)")
OTHER_OBJECTS=$(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(OTHER_C_SOURCES))

ASM_SOURCES=$(shell find $(SRC_DIR) -type f -name '*.asm' ! -path "$(SRC_DIR)/boot/*")
ASM_OBJECTS=$(patsubst $(SRC_DIR)/%.asm,$(BUILD_DIR)/%.o,$(ASM_SOURCES))

BOOT_SECTOR=$(SRC_DIR)/boot/boot.asm
BOOT_BIN=$(BUILD_DIR)/boot_sect.bin
KERNEL_ELF=$(BUILD_DIR)/kernel.elf
KERNEL_BIN=$(BUILD_DIR)/kernel.bin
OS_IMG=$(BUILD_DIR)/os.img

all: $(OS_IMG)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

check_globals:
	@missing=$(shell grep -L '#include "globals.h"' $(SRC_DIR)/**/*.c 2>/dev/null); \
	if [ "$$missing" ]; then \
		echo "HEY these files don't include globals.h:" $$missing; \
		exit 1; \
	fi

$(KERNEL_MAIN_OBJ): $(KERNEL_MAIN) | $(BUILD_DIR) check_globals
	mkdir -p $(dir $@)
	$(CC) $(C_FLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR) check_globals
	mkdir -p $(dir $@)
	$(CC) $(C_FLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.asm | $(BUILD_DIR)
	mkdir -p $(dir $@)
	$(ASM) $(NASM_FLAGS) -f elf $< -o $@

$(BOOT_BIN): $(BOOT_SECTOR) | $(BUILD_DIR)
	$(ASM) $< -f bin -I $(SRC_DIR)/boot/ -o $@

$(KERNEL_ELF): $(KERNEL_MAIN_OBJ) $(OTHER_OBJECTS) $(ASM_OBJECTS) | $(BUILD_DIR)
	$(LD) -m elf_i386 -T linker.ld -o $@ $^

$(KERNEL_BIN): $(KERNEL_ELF)
	$(OBJCOPY) -O binary $< $@

$(OS_IMG): $(BOOT_BIN) $(KERNEL_BIN)
	cat $^ > $@
	rm -f $(KERNEL_MAIN_OBJ) $(OTHER_OBJECTS) $(ASM_OBJECTS)

run: $(OS_IMG)
	qemu-system-i386 -fda $(OS_IMG)

clean:
	rm -rf $(BUILD_DIR)
