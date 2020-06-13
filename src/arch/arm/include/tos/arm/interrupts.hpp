#pragma once

#include <tos/compiler.hpp>

namespace tos::arm {
ALWAYS_INLINE
void enable_interrupts() {
    __asm__ __volatile__("cpsie i" : : : "memory");
}

ALWAYS_INLINE
void disable_interrupts() {
    __asm__ __volatile__("cpsid i" : : : "memory");
}
} // namespace tos::arm