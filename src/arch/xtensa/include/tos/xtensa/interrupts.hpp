#pragma once

#include <cstdint>
#include <tos/compiler.hpp>
#include <tos/xtensa/assembly.hpp>

#define xt_rsil(level)                                                   \
    (__extension__({                                                     \
        uint32_t state;                                                  \
        __asm__ __volatile__("rsil %0," #level : "=a"(state)::"memory"); \
        tos::xtensa::esync();                                            \
        state;                                                           \
    }))

namespace tos::xtensa {
inline bool interrupts_disabled() {
    auto ps = get_ps();
    auto int_level = ps & 0b1111;
    auto excm = ps & 0b10000;
    return excm != 0 || int_level == 15;
}

ALWAYS_INLINE void enable_interrupts() {
    xt_rsil(0);
}

ALWAYS_INLINE void disable_interrupts() {
    xt_rsil(15);
}
} // namespace tos::xtensa

#undef xt_rsil