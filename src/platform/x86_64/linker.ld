KERNEL_VMA = 4096;
ENTRY(_start)
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
    }

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
   }

   .data :
   {
        _data = .;
        *(.data*)
        *(.data.*)
        . = ALIGN(4096);
   }

   __bss_start = .;
   .bss :
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
   }

   _end = .;

   /DISCARD/ :
   {
        *(.comment)
   }
}