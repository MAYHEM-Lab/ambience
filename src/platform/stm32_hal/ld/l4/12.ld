MEMORY
{
    FLASH (rx)      : ORIGIN = 0x8000000, LENGTH = 128K
    RAM (xrw)      : ORIGIN = 0x20000000, LENGTH = 40K
}

INCLUDE stm32_common.ld
