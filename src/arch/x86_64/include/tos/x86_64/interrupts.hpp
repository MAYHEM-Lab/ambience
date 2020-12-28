#pragma once

#include <tos/compiler.hpp>

namespace tos::x86_64 {
inline bool interrupts_disabled() {
    return false;
}

ALWAYS_INLINE
void enable_interrupts() {
    asm volatile("sti" ::: "memory");
}

ALWAYS_INLINE
void disable_interrupts() {
    asm volatile("cli" ::: "memory");
}
} // namespace tos::aarch64