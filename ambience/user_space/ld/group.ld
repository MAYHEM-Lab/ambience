ENTRY(_user_code)

SECTIONS {
  __start = .;
  __text_start = .;
  .text :
  {
    KEEP(*(.text.entry))
    *(.text*)
    *(.text.*)
    . = ALIGN(4096);
  } > ROM
  __text_end = .;

  __rodata_start = .;
  .rodata :
  {
    *(.rodata*)
    *(.rodata.*)
    . = ALIGN(8);
    start_ctors = .;
    *(.init_array*)
    *(.init_array.*)
    *(.ctors*)
    *(.ctors.*)
    end_ctors = .;
    . = ALIGN(4096);
  } > ROM
  __rodata_end = .;

  .gnu_build_id :
  {
    PROVIDE(build_id = .);
    *(.note.gnu.build-id)
  } > ROM

  .ARM : {
    __exidx_start = .;
    *(.ARM.exidx*)
    __exidx_end = .;
    . = ALIGN(4096);
  } >ROM

  _sidata = LOADADDR(.data);
  .data :
  {
    __data_start = .;
    *(.data*)
    *(.data.*)
    . = ALIGN(4096);
    __data_end = .;
  } > RAM AT > ROM

  __bss_start = .;
  .bss (NOLOAD):
  {
    *(.bss*)
    *(.bss.*)
    *(COMMON)
    __bss_end = .;
    *(.nozero*);
    *(.nozero.*);
    . = ALIGN(4096);
  } > RAM

   __end = .;
}