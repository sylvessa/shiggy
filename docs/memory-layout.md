## updated 10/24/2025

| address / range   |                                               |
| ----------------- | --------------------------------------------- |
| 0x7C00            | boot sector loaded by BIOS                    |
| 0x7C00 - 0x7DFF   | boot sector code                              |
| 0x7E00 - 0x0FFFF  | bootloader stack / temporary buffers          |
| 0x20000 - 0x23FA8 | kernel `.text` section (code)                 |
| 0x23FC0 - 0x24F6C | kernel `.rodata` section (read-only data)     |
| 0x25000 - 0x250AF | kernel `.data` section (initialized globals)  |
| 0x250C0 - 0x27AA3 | kernel `.bss` section (zero-initialized data) |
