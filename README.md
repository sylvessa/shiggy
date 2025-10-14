# os

some dumb os i make for fun

to build just type `make` and then do `make run` to run on qemu

do `make iso` to turn into iso which will appear until `build/iso/`

## requiremnents

- `gcc` (32-bit support)
- `nasm`
- `ld` (GNU linker)
- `objcopy` (from binutils)
- `qemu-system-i386` (for testing)

ifu dont have ts u can just run the thing below

```bash
sudo apt update
sudo apt install build-essential nasm qemu-system
```
