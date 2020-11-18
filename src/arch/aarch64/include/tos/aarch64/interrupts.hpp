#pragma once

#include <tos/compiler.hpp>

namespace tos::aarch64 {
enum class exception_type {
    synchronous,
    irq,
    fiq,
    system_error
};

inline bool interrupts_disabled() {
    uint64_t daif;
    asm volatile ("mrs %0, daif" : "=r" (daif));
    return daif != 0;
}

ALWAYS_INLINE
void enable_interrupts() {
    asm volatile("msr daifclr, #15" ::: "memory");
}

ALWAYS_INLINE
void disable_interrupts() {
    asm volatile("msr daifset, #15" ::: "memory");
}
} // namespace tos::aarch64