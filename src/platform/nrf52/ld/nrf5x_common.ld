GROUP(-lgcc -lc -lnosys)

OUTPUT_FORMAT ("elf32-littlearm", "elf32-bigarm", "elf32-littlearm")

ENTRY(Reset_Handler)

SECTIONS
{
    .text :
    {
        KEEP(*(.isr_vector))
        *(.text*)

        KEEP(*(.init))
        KEEP(*(.fini))

        /* .ctors */
        *crtbegin.o(.ctors)
        *crtbegin?.o(.ctors)
        *(EXCLUDE_FILE(*crtend?.o *crtend.o) .ctors)
        *(SORT(.ctors.*))
        *(.ctors)

        /* .dtors */
        *crtbegin.o(.dtors)
        *crtbegin?.o(.dtors)
        *(EXCLUDE_FILE(*crtend?.o *crtend.o) .dtors)
        *(SORT(.dtors.*))
        *(.dtors)

        *(.rodata*)

        KEEP(*(.eh_frame*))

    } > FLASH

  .gnu_build_id :
  {
    PROVIDE(build_id = .);
    *(.note.gnu.build-id)
  } > FLASH

    .mem_section_dummy_rom :
      {
      }
      .sdh_ble_observers :
      {
        PROVIDE(__start_sdh_ble_observers = .);
        KEEP(*(SORT(.sdh_ble_observers*)))
        PROVIDE(__stop_sdh_ble_observers = .);
      } > FLASH
      .sdh_soc_observers :
      {
        PROVIDE(__start_sdh_soc_observers = .);
        KEEP(*(SORT(.sdh_soc_observers*)))
        PROVIDE(__stop_sdh_soc_observers = .);
      } > FLASH
      .sdh_req_observers :
      {
        PROVIDE(__start_sdh_req_observers = .);
        KEEP(*(SORT(.sdh_req_observers*)))
        PROVIDE(__stop_sdh_req_observers = .);
      } > FLASH
      .sdh_state_observers :
      {
        PROVIDE(__start_sdh_state_observers = .);
        KEEP(*(SORT(.sdh_state_observers*)))
        PROVIDE(__stop_sdh_state_observers = .);
      } > FLASH
      .sdh_stack_observers :
      {
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

    .data : AT (__etext)
    {
        __data_start__ = .;
        *(vtable)
        *(.data*)

        . = ALIGN(4);
        /* preinit data */
        PROVIDE_HIDDEN (__preinit_array_start = .);
        KEEP(*(.preinit_array))
        PROVIDE_HIDDEN (__preinit_array_end = .);

        . = ALIGN(4);
        /* init data */
        PROVIDE_HIDDEN (__init_array_start = .);
        KEEP(*(SORT(.init_array.*)))
        KEEP(*(.init_array))
        PROVIDE_HIDDEN (__init_array_end = .);

        . = ALIGN(4);
        /* finit data */
        PROVIDE_HIDDEN (__fini_array_start = .);
        KEEP(*(SORT(.fini_array.*)))
        KEEP(*(.fini_array))
        PROVIDE_HIDDEN (__fini_array_end = .);

        KEEP(*(.jcr*))
        . = ALIGN(4);
        /* All data end */
        __data_end__ = .;

    } > RAM

    .bss :
    {
        . = ALIGN(4);
        __bss_start__ = .;
        *(.bss*)
        *(COMMON)
        . = ALIGN(4);
        __bss_end__ = .;
    } > RAM

    .heap (COPY):
    {
        __HeapBase = .;
        __end__ = .;
        PROVIDE(end = .);
        KEEP(*(.heap*))
        __HeapLimit = .;
    } > RAM

    /* .stack_dummy section doesn't contains any symbols. It is only
     * used for linker to calculate size of stack sections, and assign
     * values to stack symbols later */
    .stack_dummy (COPY):
    {
        KEEP(*(.stack*))
    } > RAM

    /* Set stack top to end of RAM, and stack limit move down by
     * size of stack_dummy section */
    __StackTop = ORIGIN(RAM) + LENGTH(RAM);
    __StackLimit = __StackTop - SIZEOF(.stack_dummy);
    PROVIDE(__stack = __StackTop);

    /* Check if data + heap + stack exceeds RAM limit */
    ASSERT(__StackLimit >= __HeapLimit, "region RAM overflowed with stack")

    /* Check if text sections + data exceeds FLASH limit */
    DataInitFlashUsed = __bss_start__ - __data_start__;
    CodeFlashUsed = __etext - ORIGIN(FLASH);
    TotalFlashUsed = CodeFlashUsed + DataInitFlashUsed;
    ASSERT(TotalFlashUsed <= LENGTH(FLASH), "region FLASH overflowed with .data and user data")
}
