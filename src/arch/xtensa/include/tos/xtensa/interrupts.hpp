#pragma once

#include <cstdint>
#include <tos/compiler.hpp>

#define xt_rsil(level)                                                                   \
    (__extension__({                                                                     \
        uint32_t state;                                                                  \
        __asm__ __volatile__("rsil %0," #level ";\nesync;" : "=a"(state)::"memory");     \
        state;                                                                           \
    }))

namespace tos::xtensa {
ALWAYS_INLINE void enable_interrupts() {
    xt_rsil(0);
}

ALWAYS_INLINE void disable_interrupts() {
    xt_rsil(15);
}
} // namespace tos::xtensa

#undef xt_rsil