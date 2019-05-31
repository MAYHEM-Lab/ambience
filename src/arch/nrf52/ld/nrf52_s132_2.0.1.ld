/* Linker script to configure memory regions. */

GROUP(-lgcc -lc -lnosys)

MEMORY
{
  FLASH (rx) : ORIGIN = 0x1c000, LENGTH = 0x28800 /* Max 162 KB for dual banks */

  /* SRAM required by S132 depend on
   * - Attribute Table Size
   * - Number of concurrent connection central + peripherl + secure links
   * Several configuration with different SRAM config is included for reference
   */

  /* ATTR SIZE = 0x0B00, (central, prph, sec) = (0, 1, 0) */
  /* RAM (rwx) :  ORIGIN = 0x20001B00 + 0x0B00, LENGTH = 0xE500 - 0x0B00 */

  /* ATTR SIZE = 0x0B00, (central, prph, sec) = (4, 1, 1) */
  RAM (rwx) :  ORIGIN = 0x20001B00 + (0x0B00 + 0x1280), LENGTH = 0xE500 - (0x0B00 + 0x1280)

  /* ATTR SIZE = 0x0B00, (central, prph, sec) = (7, 1, 1) */
  /* RAM (rwx) :  ORIGIN = 0x20001B00 + (0x0B00 + 0x1F08), LENGTH = 0xE500 - (0x0B00 + 0x1F08) */
}

SECTIONS
{
  .mem_section_dummy_rom :
  {
  }
  .sdh_soc_observers :
  {
    PROVIDE(__start_sdh_soc_observers = .);
    KEEP(*(SORT(.sdh_soc_observers*)))
    PROVIDE(__stop_sdh_soc_observers = .);
  } > FLASH
  .sdh_ble_observers :
  {
    PROVIDE(__start_sdh_ble_observers = .);
    KEEP(*(SORT(.sdh_ble_observers*)))
    PROVIDE(__stop_sdh_ble_observers = .);
  } > FLASH
  .pwr_mgmt_data :
  {
    PROVIDE(__start_pwr_mgmt_data = .);
    KEEP(*(SORT(.pwr_mgmt_data*)))
    PROVIDE(__stop_pwr_mgmt_data = .);
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

} INSERT AFTER .text

INCLUDE nrf5x_common.ld

/* S132 v2.0.1 role configuration SRAM usage */
/* APP_RAM_BASE_CENTRAL_LINKS_0_PERIPH_LINKS_1_SEC_COUNT_0_MID_BW 0x20001fe8 */

/* APP_RAM_BASE_CENTRAL_LINKS_4_PERIPH_LINKS_1_SEC_COUNT_1_MID_BW 0x20003268 inc = 1280 */
/* APP_RAM_BASE_CENTRAL_LINKS_4_PERIPH_LINKS_1_SEC_COUNT_2_MID_BW 0x20003438 inc = 1450 */
/* APP_RAM_BASE_CENTRAL_LINKS_4_PERIPH_LINKS_1_SEC_COUNT_4_MID_BW 0x200037d0 inc = 17e8 */

/* APP_RAM_BASE_CENTRAL_LINKS_7_PERIPH_LINKS_1_SEC_COUNT_1_MID_BW 0x20003ef0 inc = 1F08*/
/* APP_RAM_BASE_CENTRAL_LINKS_7_PERIPH_LINKS_1_SEC_COUNT_7_MID_BW 0x200049b8 inc = 29d0*/