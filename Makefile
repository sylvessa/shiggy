ASM=nasm
CC=gcc
LD=ld
OBJCOPY=objcopy

SRC_DIR=src
BUILD_DIR=build
INC_DIR=$(SRC_DIR)/include

C_FLAGS=-fno-pic -fno-pie -fno-exceptions -ffreestanding -m32 -Wall -Wextra -I $(INC_DIR) -std=c17 -pedantic-errors -g
NASM_FLAGS=-f elf -I $(INC_DIR)

# kernel main
KERNEL_MAIN=$(SRC_DIR)/kernel/main.c
KERNEL_MAIN_OBJ=$(BUILD_DIR)/kernel/main.o

# all other C sources (drivers, libs, etc.) recursively
OTHER_C_SOURCES=$(shell find $(SRC_DIR) -type f -name '*.c' ! -path "$(KERNEL_MAIN)")
OTHER_OBJECTS=$(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(OTHER_C_SOURCES))

BOOT_SECTOR=$(SRC_DIR)/boot/boot.asm
BOOT_BIN=$(BUILD_DIR)/boot_sect.bin
KERNEL_ELF=$(BUILD_DIR)/kernel.elf
KERNEL_BIN=$(BUILD_DIR)/kernel.bin
OS_IMG=$(BUILD_DIR)/os.img

all: $(OS_IMG)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# compile kernel main
$(KERNEL_MAIN_OBJ): $(KERNEL_MAIN) | $(BUILD_DIR)
	mkdir -p $(dir $@)
	$(CC) $(C_FLAGS) -c $< -o $@

# compile other C files recursively
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	mkdir -p $(dir $@)
	$(CC) $(C_FLAGS) -c $< -o $@

# assemble boot sector
$(BOOT_BIN): $(BOOT_SECTOR) | $(BUILD_DIR)
	$(ASM) $< -f bin -I $(SRC_DIR)/boot/ -o $@

# link kernel (kernel/main.o first to guarantee entry)
$(KERNEL_ELF): $(KERNEL_MAIN_OBJ) $(OTHER_OBJECTS) | $(BUILD_DIR)
	$(LD) -m elf_i386 -o $@ -Ttext 0x1000 --entry kmain $^

# convert kernel to binary
$(KERNEL_BIN): $(KERNEL_ELF)
	$(OBJCOPY) -O binary $< $@

# final disk image
$(OS_IMG): $(BOOT_BIN) $(KERNEL_BIN)
	cat $^ > $@
	rm -f $(KERNEL_MAIN_OBJ) $(OTHER_OBJECTS)

# qemu run
run: $(OS_IMG)
	qemu-system-i386 -fda $(OS_IMG)

clean:
	rm -rf $(BUILD_DIR)
