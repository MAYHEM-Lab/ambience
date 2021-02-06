KERNEL_VMA = 0;
ENTRY(_start)
SECTIONS
{
    . = KERNEL_VMA;

    _code = .;
    .text : AT(ADDR(.text) - KERNEL_VMA)
    {
        KEEP(*(.multiboot))
        *(.text*)
        *(.text.*)
        *(.rodata*)
        *(.rodata.*)
        start_ctors = .;
        *(.ctors*)
        *(.ctors.*)
        end_ctors = .;
        *(.to_load)
    }
    . = ALIGN(4096);

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