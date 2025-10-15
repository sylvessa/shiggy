ENTRY(kmain)

SECTIONS
{
	. = 0x1000; /* load address 4 kernel */
	.text : { *(.text) }
	.rodata : { *(.rodata) }
	.data : { *(.data) }
	.bss : { *(.bss COMMON) }
	.cmds : ALIGN(4) { 
		__start_cmds = .;
		*(.cmds)
		__stop_cmds = .;
	}
}
