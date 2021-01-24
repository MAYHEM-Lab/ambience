KERNEL_VMA = 0;
ENTRY(_start)
SECTIONS
{
    . = KERNEL_VMA;

    .boot : {
        KEEP(*(.multiboot))
    }

    _code = .;
    .text : AT(ADDR(.text) - KERNEL_VMA)
    {
        *(.text*)
        *(.text.*)
        *(.rodata*)
        *(.rodata.*)
        start_ctors = .;
        *(.ctors*)
        *(.ctors.*)
        end_ctors = .;
        . = ALIGN(4096);
       *(.to_load)
    }
    . = ALIGN(4096 * 64);

   _data = .;
   .data : AT(ADDR(.data) - KERNEL_VMA)
   {
        *(.data*)
        *(.data.*)
        . = ALIGN(4096);
   }

   .eh_frame : AT(ADDR(.eh_frame) - KERNEL_VMA)
   {
       _ehframe = .;
       *(.eh_frame)
        . = ALIGN(4096);
   }

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

   _end = .;

   /DISCARD/ :
   {
        *(.comment)
   }
}