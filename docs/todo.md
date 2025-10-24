## boot/kernel

- [ ] iso booting
- [x] memory layout in a text file
- [x] virtual file system (for live testing / pre-fat32)
- [x] fat32 support **semi complete as of 10/21/2025**
- [ ] ext2 support

## drivers

- [x] keyboard support
- [x] mouse support
- [x] basic text mode
- [x] graphics mode support
- [x] sound / speaker driver
- [ ] storage drivers (support multiple ata devices, ide/sata) (wip)

## fs

- [x] fat32 initialization
- [x] root directory / cluster management (semi-complete)
- [x] file read / write
- [x] file creation
- [ ] file deletion (`rm`)
- [x] directories
- [x] file persistence (write changes to disk)
- [ ] ext2 implementation
- [x] port commands over to filesystem instead of all being in memory

## shell

- [ ] command piping / redirection (`cmd1 | cmd2`)
- [ ] environment variables (`$path`, `$home`, etc.)
- [ ] command history / autocomplete
- [ ] advanced argument parsing (quotes, escape characters)

## apps

- [ ] text editor in os (nano/vim style)
- [ ] simple calculator / utilities
- [ ] system monitor (memory, cpu usage)
- [ ] mini file manager
- [ ] ability to launch other programs

## memory / cpu

- [x] basic memory layout known
- [x] basic heap allocator (malloc/free)
- [ ] paging / virtual memory
- [ ] task scheduling / multitasking (basic threads or processes)
- [x] interrupts / isr management
- [x] timer / pit
- [ ] cpu features (apic, multi-core support)

## networking

- [ ] tcp/ip stack
- [ ] dhcp / static ip
- [ ] ping / basic networking tools
- [ ] http / ftp client/server

## gui

- [ ] window manager / gui
- [ ] mouse input integration
- [ ] basic widgets: buttons, text fields, menus

## user / security

- [ ] user system / accounts
- [ ] permissions / file ownership
- [ ] login prompt
- [ ] basic security (password hashing)

## misc

- [ ] iso boot improvements / multi-boot support
- [ ] scripting / automation support
- [ ] internal compiler / interpreter for c / scripts
- [ ] persistent configuration / settings
