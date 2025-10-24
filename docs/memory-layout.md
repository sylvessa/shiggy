## updated 10/24/2025

| address / range   |                                               |
| ----------------- | --------------------------------------------- |
| 0x7C00            | boot sector loaded by BIOS                    |
| 0x7C00 - 0x7DFF   | boot sector code                              |
| 0x7E00 - 0x0FFFF  | bootloader stack / temporary buffers          |
| 0x20000 - 0x24508 | kernel `.text` section (code)                 |
| 0x24520 - 0x24FDC | kernel `.rodata` section (read-only data)     |
| 0x26000 - 0x2608F | kernel `.data` section (initialized globals)  |
| 0x260A0 - 0x2A0C7 | kernel `.bss` section (zero-initialized data) |
