## updated 10/25/2025

| address / range   |                                               |
| ----------------- | --------------------------------------------- |
| 0x7C00            | boot sector loaded by BIOS                    |
| 0x7C00 - 0x7DFF   | boot sector code                              |
| 0x7E00 - 0x0FFFF  | bootloader stack / temporary buffers          |
| 0x20000 - 0x26F58 | kernel `.text` section (code) |
| 0x26F60 - 0x28B6C | kernel `.rodata` section (read-only data) |
| 0x29000 - 0x29123 | kernel `.data` section (initialized globals) |
| 0x29140 - 0x2C9A7 | kernel `.bss` section (zero-initialized data) |
