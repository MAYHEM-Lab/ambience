#pragma once

#include <tos/compiler.hpp>

namespace tos::arm {
inline bool interrupts_disabled() {
    uint32_t primask;
    asm volatile("mrs %0, primask" : "=r"(primask)::"memory");
    return primask == 1;
}

ALWAYS_INLINE
void enable_interrupts() {
    __asm__ __volatile__("cpsie i" : : : "memory");
}

ALWAYS_INLINE
void disable_interrupts() {
    __asm__ __volatile__("cpsid i" : : : "memory");
}
} // namespace tos::arm