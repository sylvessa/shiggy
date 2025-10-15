<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
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
=======
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

$(KERNEL_MAIN_OBJ): $(KERNEL_MAIN) | $(BUILD_DIR)
	mkdir -p $(dir $@)
	$(CC) $(C_FLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	mkdir -p $(dir $@)
	$(CC) $(C_FLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.asm | $(BUILD_DIR)
	mkdir -p $(dir $@)
	$(ASM) $(NASM_FLAGS) -f elf $< -o $@

$(BOOT_BIN): $(BOOT_SECTOR) | $(BUILD_DIR)
	$(ASM) $< -f bin -I $(SRC_DIR)/boot/ -o $@

$(KERNEL_ELF): $(KERNEL_MAIN_OBJ) $(OTHER_OBJECTS) $(ASM_OBJECTS) | $(BUILD_DIR)
	$(LD) -m elf_i386 -o $@ -Ttext 0x1000 --entry kmain $^

$(KERNEL_BIN): $(KERNEL_ELF)
	$(OBJCOPY) -O binary $< $@

$(OS_IMG): $(BOOT_BIN) $(KERNEL_BIN)
	cat $^ > $@
	rm -f $(KERNEL_MAIN_OBJ) $(OTHER_OBJECTS) $(ASM_OBJECTS)

run: $(OS_IMG)
	qemu-system-i386 -fda $(OS_IMG)

clean:
	rm -rf $(BUILD_DIR)
>>>>>>> bad254b (We got input)
=======
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

ISO_DIR=$(BUILD_DIR)/iso
ISO_IMG=$(BUILD_DIR)/os.iso

all: $(OS_IMG)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(KERNEL_MAIN_OBJ): $(KERNEL_MAIN) | $(BUILD_DIR)
	mkdir -p $(dir $@)
	$(CC) $(C_FLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	mkdir -p $(dir $@)
	$(CC) $(C_FLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.asm | $(BUILD_DIR)
	mkdir -p $(dir $@)
	$(ASM) $(NASM_FLAGS) -f elf $< -o $@

$(BOOT_BIN): $(BOOT_SECTOR) | $(BUILD_DIR)
	$(ASM) $< -f bin -I $(SRC_DIR)/boot/ -o $@

$(KERNEL_ELF): $(KERNEL_MAIN_OBJ) $(OTHER_OBJECTS) $(ASM_OBJECTS) | $(BUILD_DIR)
	$(LD) -m elf_i386 -o $@ -Ttext 0x1000 --entry kmain $^

$(KERNEL_BIN): $(KERNEL_ELF)
	$(OBJCOPY) -O binary $< $@

$(OS_IMG): $(BOOT_BIN) $(KERNEL_BIN)
	cat $^ > $@
	rm -f $(KERNEL_MAIN_OBJ) $(OTHER_OBJECTS) $(ASM_OBJECTS)

run: $(OS_IMG)
	qemu-system-i386 -fda $(OS_IMG)

clean:
	rm -rf $(BUILD_DIR)
>>>>>>> ef8ed97 (YOU CAN PRESS BACKSPACE NOW)
=======
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

ISO_DIR=$(BUILD_DIR)/iso
ISO_IMG=$(BUILD_DIR)/os.iso

all: $(OS_IMG)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# helper rule to check for globals.h
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
	$(LD) -m elf_i386 -o $@ -Ttext 0x1000 --entry kmain $^

$(KERNEL_BIN): $(KERNEL_ELF)
	$(OBJCOPY) -O binary $< $@

$(OS_IMG): $(BOOT_BIN) $(KERNEL_BIN)
	cat $^ > $@
	rm -f $(KERNEL_MAIN_OBJ) $(OTHER_OBJECTS) $(ASM_OBJECTS)

run: $(OS_IMG)
	qemu-system-i386 -fda $(OS_IMG)

clean:
	rm -rf $(BUILD_DIR)

iso: $(OS_IMG)
	mkdir -p $(ISO_DIR)
	cp $(OS_IMG) $(ISO_DIR)/os.img
	genisoimage -quiet -b os.img -no-emul-boot -boot-load-size 4 -o $(ISO_IMG) $(ISO_DIR)
>>>>>>> 8330641 ("tools" folder. i really should improve this.)
=======
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
FS_IMG=$(BUILD_DIR)/fs.img
OS_IMG=$(BUILD_DIR)/os.img

ISO_DIR=$(BUILD_DIR)/iso
ISO_IMG=$(BUILD_DIR)/os.iso

all: $(OS_IMG)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(FS_IMG): | $(BUILD_DIR)
	dd if=/dev/zero of=$(FS_IMG) bs=1K count=64
	mkfs.ext2 -F -b 1024 $(FS_IMG)
	mkdir -p /tmp/osfs
	sudo mount -o loop $(FS_IMG) /tmp/osfs
	sudo sh -c 'echo "hello from ext2" > /tmp/osfs/hello.txt && echo "test file" > /tmp/osfs/test.txt'
	sudo umount /tmp/osfs
	rm -rf /tmp/osfs

# helper rule to check for globals.h
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
	$(LD) -m elf_i386 -o $@ -Ttext 0x1000 --entry kmain $^

$(KERNEL_BIN): $(KERNEL_ELF)
	$(OBJCOPY) -O binary $< $@

$(OS_IMG): $(BOOT_BIN) $(KERNEL_BIN) $(FS_IMG)
	cat $(BOOT_BIN) $(KERNEL_BIN) $(FS_IMG) > $@
	rm -f $(KERNEL_MAIN_OBJ) $(OTHER_OBJECTS) $(ASM_OBJECTS)

run: $(OS_IMG) $(FS_IMG)
	qemu-system-i386 -fda $(OS_IMG) -fdb $(FS_IMG)

clean:
	rm -rf $(BUILD_DIR)

iso: $(OS_IMG)
	mkdir -p $(ISO_DIR)
	cp $(OS_IMG) $(ISO_DIR)/os.img
	genisoimage -quiet -b os.img -no-emul-boot -boot-load-size 4 -o $(ISO_IMG) $(ISO_DIR)
>>>>>>> f26b2a5 (ext2 testing commences)
=======
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

ISO_DIR=$(BUILD_DIR)/iso
ISO_IMG=$(BUILD_DIR)/os.iso

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
	$(LD) -m elf_i386 -o $@ -Ttext 0x1000 --entry kmain $^

$(KERNEL_BIN): $(KERNEL_ELF)
	$(OBJCOPY) -O binary $< $@

$(OS_IMG): $(BOOT_BIN) $(KERNEL_BIN)
	cat $^ > $@
	rm -f $(KERNEL_MAIN_OBJ) $(OTHER_OBJECTS) $(ASM_OBJECTS)

run: $(OS_IMG)
	qemu-system-i386 -fda $(OS_IMG)

clean:
	rm -rf $(BUILD_DIR)

iso: $(OS_IMG)
	mkdir -p $(ISO_DIR)
	cp $(OS_IMG) $(ISO_DIR)/os.img
	genisoimage -quiet -b os.img -no-emul-boot -boot-load-size 4 -o $(ISO_IMG) $(ISO_DIR)
>>>>>>> f6a2058 (Remove ext2 stuff cause im lazy rn)
