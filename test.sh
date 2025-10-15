#!/bin/bash

set -e

mkdir -p build

echo "Compiling Bootloader"
nasm -f bin -I src/boot src/boot/boot.asm -o build/boot.bin

echo "Checking and Compiling C files"

# find all .c files in src/
C_FILES=$(find src/ -name "*.c")

OBJ_FILES=""

for file in $C_FILES; do
    if ! grep -q '#include "globals.h"' "$file"; then
        echo "ERROR: $file is missing #include \"globals.h\""
        exit 1
    fi

    obj="build/$(basename "$file" .c).o"
    echo "Compiling $file -> $obj"
    i686-elf-gcc -ffreestanding -m32 -g -c "$file" -I include/ -o "$obj"
    OBJ_FILES="$OBJ_FILES $obj"
done

echo "Compiling Kernel Entry"
nasm -f elf src/boot/kernel_entry.asm -o build/entry.o
OBJ_FILES="build/entry.o $OBJ_FILES"

echo "Linking Kernel"
i686-elf-ld -o build/kernel.bin -Ttext 0x7E00 $OBJ_FILES --oformat binary

echo "Creating Disk Image"
cat build/boot.bin build/kernel.bin > build/output.img

echo "Running NovaOS"
qemu-system-x86_64 -drive file=build/output.img,format=raw
