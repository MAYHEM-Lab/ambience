#pragma once

#include <tos/compiler.hpp>

namespace tos::aarch64 {
ALWAYS_INLINE
void enable_interrupts() {
    __asm__ __volatile__("msr daifclr, #2" ::: "memory");
}

ALWAYS_INLINE
void disable_interrupts() {
    __asm__ __volatile__("msr daifset, #2" ::: "memory");
}
} // namespace tos::aarch64