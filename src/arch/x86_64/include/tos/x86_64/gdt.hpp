#pragma once

#include <cstdint>

namespace tos::x86_64 {
enum gdt_access {
    accessed = 1 << 0,
    read_write = 1 << 1,
    conform_expand_down = 1 << 2,
    code = 1 << 3,
};

struct [[gnu::packed]] gdt_entry {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t base_mid;
    uint8_t access;
    uint8_t opts_limit_mid;
    uint8_t base_hi;

    gdt_entry& zero() {
        *this = {};
        return *this;
    }

    gdt_entry& base(uint32_t addr) {
        base_low = addr & 0xFFFF;
        base_mid = (addr >> 16) & 0xFF;
        base_hi = (addr >> 24) & 0xFF;
        return *this;
    }

    gdt_entry& limit(uint32_t lim) {
        limit_low = lim & 0xFFFF;
        opts_limit_mid = (opts_limit_mid & 0xF0) | ((lim >> 16) & 0xF);
        return *this;
    }

    gdt_entry& type(uint8_t type) {
        access = type;
        return *this;
    }
};
} // namespace tos::x86_64