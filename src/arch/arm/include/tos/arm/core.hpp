#pragma once

#if defined(SCB)
#undef SCB
#endif

#if defined(DWT)
#undef DWT
#endif

#include <tos/mmio/reg.hpp>

namespace tos::arm {
inline bool in_handler_mode() {
    constexpr mmio::reg<0xE000ED04, uint32_t> ICSR;
    return (ICSR.read() & 0x1FF) != 0;
}

struct DWT {
    static constexpr mmio::reg<0xE0001000, uint32_t> CONTROL{};
    static constexpr mmio::reg<0xE0001004, uint32_t> CYCCNT{};
    static constexpr mmio::reg<0xE0001FB0, uint32_t> LAR{};
};

struct SCB {
    static constexpr mmio::reg<0xE000EDFC, uint32_t> DEMCR{};
};
}
