ENTRY(_user_code)

SECTIONS {
    __start = .;
    __text_start = .;
    .text :
    {
        KEEP(*(.text.entry))
        *(.text*)
        *(.text.*)
    } > RODATA
    __text_end = .;

    __rodata_start = .;
   .rodata :
   {
        *(.rodata*)
        *(.rodata.*)
        start_ctors = .;
        *(.init_array*)
        *(.init_array.*)
        *(.ctors*)
        *(.ctors.*)
        end_ctors = .;
   } > RODATA
   . = ALIGN(4096);
   __rodata_end = .;

  .ARM : {
    __exidx_start = .;
    *(.ARM.exidx*)
    __exidx_end = .;
  } >RODATA

  _sidata = LOADADDR(.data);
   .data :
   {
   __data_start = .;
        *(.data*)
        *(.data.*)
   . = ALIGN(4096);
   __data_end = .;
   } > DATA AT > RODATA

   __bss_start = .;
   .bss (NOLOAD):
   {
       *(.bss*)
       *(.bss.*)

        *(COMMON)
        __bss_end = .;
       *(.nozero*);
       *(.nozero.*);
   } > DATA
   . = ALIGN(4096);

   __end = .;
}