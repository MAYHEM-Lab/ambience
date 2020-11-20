#pragma once

#include <cstdint>
#include <tos/compiler.hpp>

namespace tos::xtensa {
ALWAYS_INLINE
void nop() {
    asm volatile("nop");
}

inline void esync() {
    asm volatile("esync" : : : "memory");
}

inline void rsync() {
    asm volatile("rsync" : : : "memory");
}

inline uint32_t get_exccause() {
    uint32_t res;
    asm volatile("rsr %0, EXCCAUSE" : "=r"(res) : :);
    return res;
}

inline uint32_t get_ps() {
    uint32_t res;
    asm volatile("rsr %0, PS" : "=r"(res) : :);
    return res;
}
}