ENTRY(kmain)
SECTIONS
{
	. = 0x1000;
	.text : { *(.text) }
	.rodata : { *(.rodata) }
	.data : { *(.data) }

	.cmds : ALIGN(4)
    {
        __start_cmds = .;
        *(.cmds)
        /* reserve extra space in case more commands are added later */
        __stop_cmds = .;
    }

	.bss : { *(.bss COMMON) }
}
