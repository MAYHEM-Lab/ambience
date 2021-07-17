KERNEL_VMA = 0x200000;
ENTRY(_start)

PHDRS
{
  RODATA PT_LOAD FLAGS(5);
  DATA PT_LOAD;
}

SECTIONS
{
    . = KERNEL_VMA;

    .stivale2hdr ALIGN(4K) : {
        KEEP(*(.stivale2hdr))
    }

    __start = .;
    __text_start = .;
    .text :
    {
        KEEP(*(.text.entry))
        *(.text*)
        *(.text.*)
        . = ALIGN(4096);
    } : RODATA
    __text_end = .;

    __rodata_start = .;
   .rodata :
   {
        _rodata = .;
        *(.rodata*)
        *(.rodata.*)
        .  = ALIGN(8);
        start_ctors = .;
        *(.init_array*)
        *(.init_array.*)
        *(.ctors*)
        *(.ctors.*)
        end_ctors = .;
        *(.to_load)
        *(.to_load.*)
        . = ALIGN(4096);
   } : RODATA
   __rodata_end = .;

  .gnu_build_id :
  {
    PROVIDE(build_id = .);
    *(.note.gnu.build-id)
  } : RODATA

    . = ALIGN(4096);
   __data_start = .;
   .data :
   {
        _data = .;
        *(.data*)
        *(.data.*)
        . = ALIGN(4096);
   } : DATA
   __data_end = .;

   __bss_start = .;
   .bss (NOLOAD):
   {
       _bss = .;
       *(.bss*)
       *(.bss.*)

       /*
        * You usually need to include generated COMMON symbols
        * under kernel BSS section or use gcc's -fno-common
        */

        *(COMMON)
        __bss_end = .;
       *(.nozero*);
       *(.nozero.*);
       . = ALIGN(4096);
   } : DATA

   __end = .;

   /DISCARD/ :
   {
        *(.comment)
   }
}