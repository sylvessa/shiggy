ENTRY(kmain)
SECTIONS
{
	. = 0x1000;

	.text : { *(.text) }

	.rodata : { *(.rodata) }

	.data : {
		*(.data)
		. += 0x4000;  /* reserve extra 16KB for global variables */
	}

	.cmds : ALIGN(4) {
		__start_cmds = .;
		*(.cmds)
		. += 0x1000;  /* reserve extra 4KB for commands */
		__stop_cmds = .;
	}

	.bss : { 
		*(.bss COMMON)
		. += 0x8000;  /* reserve extra 32KB for uninitialized globals */
	}
}
