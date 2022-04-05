ENTRY(g_pfnVectors)

PHDRS {
  RODATA PT_LOAD FLAGS(5);
  DATA PT_LOAD;
}

SECTIONS
{
  .isr_vector :
  {
    . = ALIGN(4);
    KEEP(*(.isr_vector))
    . = ALIGN(4);
  } >FLASH : RODATA

  .text :
  {
    . = ALIGN(4);
    *(.text)
    *(.text*)
    . = ALIGN(4);
    _etext = .;
  } >FLASH

  .gnu_build_id :
  {
    PROVIDE(build_id = .);
    *(.note.gnu.build-id)
  } > FLASH

  .rodata :
  {
    . = ALIGN(4);
    _estack = ORIGIN(RAM) + LENGTH(RAM);
    *(.rodata)
    *(.rodata*)
    . = ALIGN(4);
    PROVIDE(start_group_inits = .);
    KEEP(*(SORT(.group_init*)))
    PROVIDE(end_group_inits = .);
    . = ALIGN(4);
    *(.to_load)
    . = ALIGN(4);
  } >FLASH

  .ARM : {
    __exidx_start = .;
    *(.ARM.exidx*)
    __exidx_end = .;
  } >FLASH

  .init_array :
  {
    PROVIDE_HIDDEN (start_ctors = .);
    KEEP (*(SORT(.init_array.*)))
    KEEP (*(.init_array*))
    PROVIDE_HIDDEN (end_ctors = .);
  } >FLASH

  _sidata = LOADADDR(.data);

  .data :
  {
    . = ALIGN(4);
    __data_start = .;
    *(.data)
    *(.data*)

    . = ALIGN(4);
    __data_end = .;
  } > RAM
  AT> FLASH : DATA

  .nozero (NOLOAD) : {
    *(.nozero*)
  } >RAM : DATA

  /* Uninitialized data section */
  . = ALIGN(4);
  .bss (NOLOAD) :
  {
    __bss_start = .;
    *(.bss)
    *(.bss*)
    *(COMMON)

    . = ALIGN(4);
    __bss_end = .;
  } >RAM : DATA

  PROVIDE (end = .);
  PROVIDE ( _end = . );
}
