MEMORY {
    ROM(rx) : ORIGIN = 0, LENGTH = 1024M
    RAM(rw) : ORIGIN = 0x40000000, LENGTH = 1024M
}

INCLUDE group.ld