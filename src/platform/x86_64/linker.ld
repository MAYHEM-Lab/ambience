KERNEL_VMA = 4096;
ENTRY(_start)
SECTIONS
{
    . = KERNEL_VMA;

    _code = .;
    .text : AT(ADDR(.text) - KERNEL_VMA)
    {
        KEEP(*(.text.entry))
        *(.text*)
        *(.text.*)
        . = ALIGN(4096);
    }

   .rodata : AT(ADDR(.rodata) - KERNEL_VMA)
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
   }

   .data : AT(ADDR(.data) - KERNEL_VMA)
   {
        _data = .;
        *(.data*)
        *(.data.*)
        . = ALIGN(4096);
   }

   __bss_start = .;
   .bss : AT(ADDR(.bss) - KERNEL_VMA)
   {
       _bss = .;
       *(.bss*)
       *(.bss.*)

       /*
        * You usually need to include generated COMMON symbols
        * under kernel BSS section or use gcc's -fno-common
        */

        *(COMMON)
       . = ALIGN(4096);
   }
   __bss_end = .;

   _end = .;

   /DISCARD/ :
   {
        *(.comment)
   }
}