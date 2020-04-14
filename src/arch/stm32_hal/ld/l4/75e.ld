MEMORY
{
    FLASH (rx)      : ORIGIN = 0x8000000, LENGTH = 1024K
    RAM (xrw)      : ORIGIN = 0x20000000, LENGTH = 96K
}

INCLUDE stm32_common.ld
