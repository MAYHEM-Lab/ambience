MEMORY
{
  FLASH_SD (r) :    ORIGIN = 0,             LENGTH = 152K
  FLASH (rx) :      ORIGIN = 0x00026000,    LENGTH = 872K
  RAM (rwx) :       ORIGIN = 0x20010058,    LENGTH = 0x2FFA8
}

INCLUDE "nrf5x_common.ld"
