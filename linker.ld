ENTRY(kmain)
SECTIONS
{
	. = 0x1000;

	.text : {
        *(.text.unlikely .text.*_unlikely .text.unlikely.*)
        *(.text.exit .text.exit.*)
        *(.text.startup .text.startup.*)
        *(.text.hot .text.hot.*)
        *(.text .stub .text.* .gnu.linkonce.t.*)
    }
    PROVIDE(etext = .);

    .cmds : ALIGN(4) {
        __start_cmds = .;
        *(.cmds)
        __stop_cmds = .;
    }
    
    .rodata : {
        *(.rodata .rodata.* .gnu.linkonce.r.*)
        *(.data.rel.ro.local* .gnu.linkonce.d.rel.ro.local.*)
        *(.data.rel.ro .data.rel.ro.* .gnu.linkonce.d.rel.ro.*)
    }

    . = ALIGN(4096);
    PROVIDE(start_data = .);
    .data : {
        *(.data .data.* .gnu.linkonce.d.*)
        SORT(CONSTRUCTORS)
    }
    PROVIDE(edata = .);

    .bss : {
        *(.bss .bss.* .gnu.linkonce.b.*)
    }
    PROVIDE(end = .);

    /DISCARD/ : { *(.eh_frame .note.GNU-stack) }
}
