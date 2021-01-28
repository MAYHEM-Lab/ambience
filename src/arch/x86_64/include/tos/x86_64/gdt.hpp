#pragma once

#include <cstdint>

namespace tos::x86_64 {
struct [[gnu::packed]] gdt_entry {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t base_mid;
    uint8_t access;
    uint8_t opts_limit_mid;
    uint8_t base_hi;
};
} // namespace tos::x86_64