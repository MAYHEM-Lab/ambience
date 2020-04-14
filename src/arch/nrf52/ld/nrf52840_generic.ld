MEMORY
{
  FLASH_SD (r) :    ORIGIN = 0,             LENGTH = 156K
  FLASH (rx) : ORIGIN = 0x27000, LENGTH = 0xd9000
    RAM (rwx) :  ORIGIN = 0x20002ae8, LENGTH = 0x3d518
}


INCLUDE "nrf5x_common.ld"
