MEMORY
{
  FLASH (rx) : ORIGIN = 0, LENGTH = 1M
  RAM (rwx) :  ORIGIN = 0x20000000, LENGTH = 256K
}


INCLUDE "nrf5x_nosd.ld"
