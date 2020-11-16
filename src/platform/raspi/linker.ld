ENTRY(_start)
 
SECTIONS
{
    /* Starts at LOADER_ADDR. */
    . = 0x80000;
    __start = .;
    __text_start = .;
    .text :
    {
        KEEP(*(.text.boot))
        KEEP(*(.text.vectors))
        *(.text.*)
        *(.text*)
    }
    . = ALIGN(4096); /* align to page size */
    __text_end = .;
 
    __rodata_start = .;
    .rodata :
    {
        PROVIDE(start_ctors = .);
        KEEP(*(.init_array))
        PROVIDE(end_ctors = .);
        *(.rodata.*)
        *(.rodata)
    }
    .gnu_build_id :
    {
        PROVIDE(build_id = .);
        *(.note.gnu.build-id)
    }
    . = ALIGN(4096); /* align to page size */
    __rodata_end = .;

    __data_start = .;
    .data :
    {
        *(.data.*)
        *(.data*)
    }
    . = ALIGN(4096); /* align to page size */
    __data_end = .;
 
    PROVIDE(__bss_start = .);
    .bss :
    {
        *(.bss.*)
        *(.bss*)
    }
    . = ALIGN(4096); /* align to page size */
    PROVIDE(__bss_end = .);
    PROVIDE(__bss_size = __bss_end - __bss_start);
    __end = .;
}