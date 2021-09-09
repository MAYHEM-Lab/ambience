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

      .sdh_observers :
      {
        PROVIDE(__start_sdh_ble_observers = .);
        KEEP(*(SORT(.sdh_ble_observers*)))
        PROVIDE(__stop_sdh_ble_observers = .);
        PROVIDE(__start_sdh_soc_observers = .);
        KEEP(*(SORT(.sdh_soc_observers*)))
        PROVIDE(__stop_sdh_soc_observers = .);
        PROVIDE(__start_sdh_req_observers = .);
        KEEP(*(SORT(.sdh_req_observers*)))
        PROVIDE(__stop_sdh_req_observers = .);
        PROVIDE(__start_sdh_state_observers = .);
        KEEP(*(SORT(.sdh_state_observers*)))
        PROVIDE(__stop_sdh_state_observers = .);
        PROVIDE(__start_sdh_stack_observers = .);
        KEEP(*(SORT(.sdh_stack_observers*)))
        PROVIDE(__stop_sdh_stack_observers = .);
      } > FLASH
        .nrf_queue :
      {
        PROVIDE(__start_nrf_queue = .);
        KEEP(*(.nrf_queue))
        PROVIDE(__stop_nrf_queue = .);
      } > FLASH
        .nrf_balloc :
      {
        PROVIDE(__start_nrf_balloc = .);
        KEEP(*(.nrf_balloc))
        PROVIDE(__stop_nrf_balloc = .);
      } > FLASH
        .cli_command :
      {
        PROVIDE(__start_cli_command = .);
        KEEP(*(.cli_command))
        PROVIDE(__stop_cli_command = .);
      } > FLASH
      .crypto_data :
      {
        PROVIDE(__start_crypto_data = .);
        KEEP(*(SORT(.crypto_data*)))
        PROVIDE(__stop_crypto_data = .);
      } > FLASH
      .pwr_mgmt_data :
      {
        PROVIDE(__start_pwr_mgmt_data = .);
        KEEP(*(SORT(.pwr_mgmt_data*)))
        PROVIDE(__stop_pwr_mgmt_data = .);
      } > FLASH
      .log_const_data :
      {
        PROVIDE(__start_log_const_data = .);
        KEEP(*(SORT(.log_const_data*)))
        PROVIDE(__stop_log_const_data = .);
      } > FLASH
      .log_backends :
      {
        PROVIDE(__start_log_backends = .);
        KEEP(*(SORT(.log_backends*)))
        PROVIDE(__stop_log_backends = .);
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
