#pragma once

#include <tos/compiler.hpp>

namespace tos::aarch64 {
enum class exception_type {
    synchronous,
    irq,
    fiq,
    system_error
};

ALWAYS_INLINE
void enable_interrupts() {
    __asm__ __volatile__("msr daifclr, #15" ::: "memory");
}

ALWAYS_INLINE
void disable_interrupts() {
    __asm__ __volatile__("msr daifset, #15" ::: "memory");
}
} // namespace tos::aarch64