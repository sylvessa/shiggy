## updated 10/26/2025

| address / range   |                                               |
| ----------------- | --------------------------------------------- |
| 0x7C00            | boot sector loaded by BIOS                    |
| 0x7C00 - 0x7DFF   | boot sector code                              |
| 0x7E00 - 0x0FFFF  | bootloader stack / temporary buffers          |
| 0x20000 - 0x27308 | kernel `.text` section (code)                 |
| 0x27320 - 0x2902C | kernel `.rodata` section (read-only data)     |
| 0x29030 - 0x291C7 | kernel `.cmds` section (in-built commands)    |
| 0x2A000 - 0x2A123 | kernel `.data` section (initialized globals)  |
| 0x2A140 - 0x2DBA7 | kernel `.bss` section (zero-initialized data) |
