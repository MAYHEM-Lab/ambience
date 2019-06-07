MEMORY
{
	rom (rx)  : ORIGIN = 0x08000000, LENGTH = 1024K
	ram (rwx)   : ORIGIN = 0x20000000, LENGTH = 96K
    RAM_2 (rwx) : ORIGIN = 0x10000000, LENGTH = 32K
}

INCLUDE cortex-m-generic.ld
