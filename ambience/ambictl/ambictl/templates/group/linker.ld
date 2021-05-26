MEMORY {
    ROM(rx) : ORIGIN = {{rom_base}}, LENGTH = {{rom_size}}
    RAM(rw) : ORIGIN = {{ram_base}}, LENGTH = {{ram_size}}
}

INCLUDE group.ld