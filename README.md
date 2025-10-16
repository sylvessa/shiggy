# os

some dumb os i make for fun

to build just type `make` and then do `make run` to run on qemu

do `make iso` to turn into iso which will appear until `build/iso/`

**anything before 10/15/2025 was screwed up, this project actually started 10/13/2025**

## requiremnents

- `gcc` (32-bit support)
- `nasm`
- `ld` (GNU linker)
- `objcopy` (from binutils)
- `qemu-system-i386` (for testing)

if u dont have ts u can just run the thing below

```bash
sudo apt update
sudo apt install build-essential nasm qemu-system
```

Why not i386-elf-gcc etc? Because I dont want to make people have to build a toolset just for this project.

I have a lot of plans for this project.
