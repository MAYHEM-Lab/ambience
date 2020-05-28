#pragma once

#include <tos/barrier.hpp>

namespace tos::arm {
inline void breakpoint() {
    asm volatile("bkpt 0");
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

inline void dmb() {
    tos::detail::memory_barrier();
    asm volatile("dmb");
    tos::detail::memory_barrier();
}

inline void nop() {
    asm volatile("nop");
}
} // namespace tos::arm