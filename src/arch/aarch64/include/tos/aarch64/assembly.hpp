#pragma once

#include <tos/barrier.hpp>

namespace tos::aarch64 {
inline void nop() {
    asm volatile("nop");
}

inline void breakpoint() {
    asm volatile("brk #0");
}

inline void isb() {
    tos::detail::memory_barrier();
    asm volatile("isb");
    tos::detail::memory_barrier();
}

inline void dsb() {
    tos::detail::memory_barrier();
    asm volatile("dsb");
    tos::detail::memory_barrier();
}
} // namespace tos::aarch64