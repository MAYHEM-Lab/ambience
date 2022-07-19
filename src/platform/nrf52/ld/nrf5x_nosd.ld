ENTRY(g_pfnVectors)

PHDRS {
  RODATA PT_LOAD FLAGS(5);
  DATA PT_LOAD;
}

SECTIONS
{
    .text :
    {
        KEEP(*(.isr_vector*))
        *(.text*)

        . = ALIGN(4);
        PROVIDE(start_ctors = .);
        KEEP(*(SORT(.init_array.*)))
        KEEP(*(.init_array))
        PROVIDE(end_ctors = .);
        PROVIDE(start_group_inits = .);
        KEEP(*(SORT(.group_init*)))
        PROVIDE(end_group_inits = .);

    _estack = ORIGIN(RAM) + LENGTH(RAM);
        *(.rodata*)
        *(vtable)

        KEEP(*(.eh_frame*))

    } > FLASH : RODATA

  .gnu_build_id :
  {
    PROVIDE(build_id = .);
    *(.note.gnu.build-id)
  } > FLASH

      .pwr_mgmt_data :
      {
        PROVIDE(__start_pwr_mgmt_data = .);
        KEEP(*(SORT(.pwr_mgmt_data*)))
        PROVIDE(__stop_pwr_mgmt_data = .);
      } > FLASH
      
    .ARM.extab :
    {
        *(.ARM.extab* .gnu.linkonce.armextab.*)
    } > FLASH

    __exidx_start = .;
    .ARM.exidx :
    {
        *(.ARM.exidx* .gnu.linkonce.armexidx.*)
    } > FLASH
    __exidx_end = .;

    __etext = .;


  _sidata = LOADADDR(.data);

  .data :
  {
    . = ALIGN(4);
    __data_start__ = .; /* Used by the NRF SDK */
    __data_start = .;
    *(.data)
    *(.data*)

    . = ALIGN(4);
    __data_end = .;
  } > RAM
  AT> FLASH : DATA

  .nozero (NOLOAD) : {
    *(.nozero*)
  } >RAM

  /* Uninitialized data section */
  . = ALIGN(4);
  .bss (NOLOAD) :
  {
    _sbss = .;
    __bss_start__ = _sbss;
    __bss_start = _sbss;
    *(.bss)
    *(.bss*)
    *(COMMON)

    . = ALIGN(4);
    __bss_end = .;
  } >RAM

  PROVIDE (end = .);
  PROVIDE ( _end = . );
}
