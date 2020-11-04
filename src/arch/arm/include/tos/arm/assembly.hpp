#pragma once

#include <tos/barrier.hpp>
#include <cstdint>

namespace tos::arm {
inline void breakpoint() {
    asm volatile("bkpt 0");
}

inline void svc1() {
    asm volatile("svc 1");
}

inline void svc127() {
    asm volatile("svc 127");
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

inline void set_control(uint32_t val) {
    asm volatile("msr CONTROL, %0" : : "r"(val) : "memory");
}

inline uint32_t get_control() {
    uint32_t res;
    asm volatile("mrs %0, CONTROL" : "=r"(res));
    return res;
}
} // namespace tos::arm