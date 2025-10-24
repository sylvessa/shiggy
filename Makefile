ASM=nasm
CC=i386-elf-gcc
LD=i386-elf-ld
OBJCOPY=objcopy

SRC_DIR=src
BUILD_DIR=build
INC_DIR=include

C_FLAGS=-fno-pic -fno-pie -fno-exceptions -ffreestanding -fno-stack-protector \
	-nostdlib -nostdinc -m32 -Wall -Wextra -Wno-unused-parameter \
	-I $(INC_DIR) -std=c17 -pedantic-errors -g
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
HDD_IMG=$(BUILD_DIR)/hdd.img

SECTOR_SIZE=512
KERNEL_SECTORS=$(shell if [ -f $(KERNEL_BIN) ]; then stat -c%s $(KERNEL_BIN) | awk -v sz=$(SECTOR_SIZE) '{printf "%d", ($$1+sz-1)/sz}'; else echo 50; fi)

GREEN=\033[0;32m
BLUE=\033[0;34m
CYAN=\033[0;36m
YELLOW=\033[1;33m
RESET=\033[0m

all: $(OS_IMG)

$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

check_globals:
	@missing=$(shell grep -L '#include "globals.h"' $(SRC_DIR)/**/*.c 2>/dev/null); \
	if [ "$$missing" ]; then \
		echo "$(YELLOW)HEY these files don't include globals.h:$(RESET) $$missing"; \
		exit 1; \
	fi

$(KERNEL_MAIN_OBJ): $(KERNEL_MAIN) | $(BUILD_DIR) check_globals
	@mkdir -p $(dir $@)
	@echo "$(GREEN)[CC]$(RESET) $<"
	@$(CC) $(C_FLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR) check_globals
	@mkdir -p $(dir $@)
	@echo "$(GREEN)[CC]$(RESET) $<"
	@$(CC) $(C_FLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.asm | $(BUILD_DIR)
	@mkdir -p $(dir $@)
	@echo "$(CYAN)[ASM]$(RESET) $<"
	@$(ASM) $(NASM_FLAGS) -f elf $< -o $@

$(BOOT_BIN): $(BOOT_SECTOR) | $(BUILD_DIR)
	@echo "$(CYAN)[BOOT]$(RESET) $< Setting sectors to $(KERNEL_SECTORS)"
	@$(ASM) $< -DNUM_KERNEL_SECTORS=$(KERNEL_SECTORS) -f bin -I $(SRC_DIR)/boot/ -o $@

$(KERNEL_ELF): $(KERNEL_MAIN_OBJ) $(OTHER_OBJECTS) $(ASM_OBJECTS) | $(BUILD_DIR)
	@echo "$(BLUE)[LD]$(RESET) linking $@"
	@$(LD) -m elf_i386 -T linker.ld -o $@ $^ -Map $(BUILD_DIR)/kernel.map

$(KERNEL_BIN): $(KERNEL_ELF)
	@echo "$(BLUE)[OBJCOPY]$(RESET) $<"
	@$(OBJCOPY) -O binary $< $@

$(OS_IMG): $(BOOT_BIN) $(KERNEL_BIN)
	@echo "$(YELLOW)[CAT]$(RESET) building disk image"
	@cat $^ > $@
	@rm -f $(KERNEL_MAIN_OBJ) $(OTHER_OBJECTS) $(ASM_OBJECTS)
	@echo "$(GREEN)[OK]$(RESET) built $(OS_IMG)"

$(HDD_IMG):
	@echo "$(YELLOW)[HDD]$(RESET) creating blank IDE hard drive image"
	@dd if=/dev/zero of=$(HDD_IMG) bs=1M count=64 status=none
	@echo "$(GREEN)[OK]$(RESET) created $(HDD_IMG)"

run: $(OS_IMG) $(HDD_IMG)
	@echo "$(CYAN)[QEMU]$(RESET) running with hdd..."
	@qemu-system-i386 -drive file=$(OS_IMG),format=raw,if=floppy -drive file=$(HDD_IMG),format=raw,if=ide -boot a

run-nb:
	@echo "$(CYAN)[QEMU]$(RESET) running with hdd..."
	@qemu-system-i386 -drive file=$(OS_IMG),format=raw,if=floppy -drive file=$(HDD_IMG),format=raw,if=ide -boot a

clean:
	@echo "$(YELLOW)[CLEAN]$(RESET) removing build dir"
	@rm -rf $(BUILD_DIR)
