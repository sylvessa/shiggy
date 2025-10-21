## updated 10/21/2025

| address / range   |                                               |
| ----------------- | --------------------------------------------- |
| 0x7C00            | boot sector loaded by BIOS                    |
| 0x7C00 - 0x7DFF   | boot sector code                              |
| 0x7E00 - 0x0FFFF  | bootloader stack / temporary buffers          |
| 0x20000 - 0x236A8 | kernel `.text` section (code)                 |
| 0x236C0 - 0x23F70 | kernel `.rodata` section (read-only data)     |
| 0x24000 - 0x24147 | kernel `.data` section (initialized globals)  |
| 0x24148 - 0x2416B | kernel `.cmds` section (command table)        |
| 0x24180 - 0x2662F | kernel `.bss` section (zero-initialized data) |
