MEMORY
{
  FLASH (rx) : ORIGIN = 0, LENGTH = 512K
  RAM (rwx) :  ORIGIN = 0x20000000, LENGTH = 64K
}


INCLUDE "nrf5x_nosd.ld"
