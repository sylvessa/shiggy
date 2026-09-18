## updated 09/18/2026

| address / range   |                                               |
| ----------------- | --------------------------------------------- |
| 0x7C00            | boot sector loaded by BIOS                    |
| 0x7C00 - 0x7DFF   | boot sector code                              |
| 0x7E00 - 0x0FFFF  | bootloader stack / temporary buffers          |
| 0x20000 - 0x29918 | kernel `.text` section (code) |
| 0x29920 - 0x2B41B | kernel `.rodata` section (read-only data) |
| 0x2B41C - 0x2B5D3 | kernel `.cmds` section (in-built commands) |
| 0x2C000 - 0x2C11F | kernel `.data` section (initialized globals) |
| 0x2C120 - 0x33CA5 | kernel `.bss` section (zero-initialized data) |
