#include <cstdint>

[[gnu::section(".to_load")]] uint8_t program[] = {
#include <program.data>
};