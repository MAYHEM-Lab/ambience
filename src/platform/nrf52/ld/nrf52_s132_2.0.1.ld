MEMORY
{
  FLASH_SD (r) :    ORIGIN = 0,             LENGTH = 152K
  FLASH (rx) :      ORIGIN = 0x00026000,    LENGTH = 360K
  RAM (rwx)  :      ORIGIN = 0x20003AE8,    LENGTH = 0x20010000 - 0x20003AE8
}

INCLUDE nrf5x_common.ld