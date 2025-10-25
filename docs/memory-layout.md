## updated 10/25/2025

| address / range   |                                               |
| ----------------- | --------------------------------------------- |
| 0x7C00            | boot sector loaded by BIOS                    |
| 0x7C00 - 0x7DFF   | boot sector code                              |
| 0x7E00 - 0x0FFFF  | bootloader stack / temporary buffers          |
| 0x20000 - 0x27518 | kernel `.text` section (code)                 |
| 0x27520 - 0x291AC | kernel `.rodata` section (read-only data)     |
| 0x2A000 - 0x2A123 | kernel `.data` section (initialized globals)  |
| 0x2A140 - 0x2D9A7 | kernel `.bss` section (zero-initialized data) |
