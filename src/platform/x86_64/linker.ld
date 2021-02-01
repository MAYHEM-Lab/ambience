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

    _code = .;
    .text :
    {
        KEEP(*(.text.entry))
        *(.text*)
        *(.text.*)
        . = ALIGN(4096);
    } : RODATA

   .rodata :
   {
        _rodata = .;
        *(.rodata*)
        *(.rodata.*)
        start_ctors = .;
        *(.init_array*)
        *(.init_array.*)
        *(.ctors*)
        *(.ctors.*)
        end_ctors = .;
        . = ALIGN(4096);
   } : RODATA

   .data :
   {
        _data = .;
        *(.data*)
        *(.data.*)
        . = ALIGN(4096);
   } : DATA

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

   _end = .;

   /DISCARD/ :
   {
        *(.comment)
   }
}