## updated 09/18/2026

| address / range   |                                               |
| ----------------- | --------------------------------------------- |
| 0x7C00            | boot sector loaded by BIOS                    |
| 0x7C00 - 0x7DFF   | boot sector code                              |
| 0x7E00 - 0x0FFFF  | bootloader stack / temporary buffers          |
| 0x20000 - 0x2BCB8 | kernel `.text` section (code) |
| 0x2BCC0 - 0x2D75B | kernel `.rodata` section (read-only data) |
| 0x2D75C - 0x2D913 | kernel `.cmds` section (in-built commands) |
| 0x2E000 - 0x2E13F | kernel `.data` section (initialized globals) |
| 0x2E140 - 0x39A45 | kernel `.bss` section (zero-initialized data) |
