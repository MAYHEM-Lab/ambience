#pragma once

#include <cstdint>

namespace tos::x86_64 {
enum gdt_access
{
    accessed = 1 << 0,
    read_write = 1 << 1,
    conform_expand_down = 1 << 2,
    code = 1 << 3,
};

struct [[gnu::packed]] gdt_entry {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t base_mid;

    // 7 -> present
    // [5-6] -> DPL
    // 4 -> S field
    // [0-3] -> type
    uint8_t access;

    // 7 -> granularity, always ignored
    // 6 -> D is always 0 for 64 bit
    // 5 -> Long mode
    // 4 -> Available for use
    // [0-3] -> limit mid
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

    gdt_entry& long_mode(bool val) {
        opts_limit_mid = (opts_limit_mid & ~long_mode_mask) | val << long_mode_pos;
        return *this;
    }

    bool long_mode() const {
        return opts_limit_mid & long_mode_mask;
    }

    gdt_entry& type(uint8_t type) {
        access = type;
        return *this;
    }

    static constexpr auto dpl_pos = 1;
    static constexpr auto dpl_mask = 0b11 << dpl_pos;

    static constexpr auto long_mode_pos = 5;
    static constexpr auto long_mode_mask = 1 << long_mode_pos;
};

struct [[gnu::packed]] expanded_gdt_entry {
    gdt_entry entry;
    uint32_t base_higher;
    uint32_t reserved = 0;

    expanded_gdt_entry& zero() {
        *this = {};
        return *this;
    }

    expanded_gdt_entry& base(uint64_t addr) {
        entry.base(addr & 0xFFFFFFFF);
        base_higher = (addr >> 32) & 0xFFFFFFFF;
        return *this;
    }
};
} // namespace tos::x86_64