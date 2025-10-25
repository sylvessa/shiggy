# shiggy

an experimental OS I made for fun.

shiggy is written in C with a bit of x86 assembly. theres is a work in progress file system, this is just the kernel and bootloader all handcrafted.

you can build and run it with:

```bash
make
make run
```

this should also run and build just fine on WSL2.

## showcase 10/25/2025

<img src="https://raw.githubusercontent.com/sylvessa/sylvessa/refs/heads/main/msrdc_vQ8qDDAxYO.gif">

## features

- entirely custom kernel and bootloader
- written in C with some x86 assembly
- includes a built-in command system
- fat32 file system support

## requirements

- `i386-elf-gcc`
- `nasm`
- `i386-elf-ld`
- `objcopy` (from binutils)
- `qemu-system-i386` (for testing)

if you dont have them installed:

```bash
sudo apt update
sudo apt install build-essential nasm qemu-system
# i386-elf tools requires being built manually, you can find tutorials online for it, i will provide a script for it later
```

## info

the OS is very barebones, but includes a set of commands you can explore. its designed to experiment with kernel level programming and just to practice my programming skills, so everything from bootloader to kernel functions is made from scratch.

<img src="https://raw.githubusercontent.com/sylvessa/sylvessa/refs/heads/main/caption.gif" width="250" height="250">
