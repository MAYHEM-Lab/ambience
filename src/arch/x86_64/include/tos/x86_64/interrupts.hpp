#pragma once

#include <tos/compiler.hpp>
#include <tos/x86_64/assembly.hpp>

namespace tos::x86_64 {
ALWAYS_INLINE
bool interrupts_disabled() {
    return !(read_flags() & (1 << 9));
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