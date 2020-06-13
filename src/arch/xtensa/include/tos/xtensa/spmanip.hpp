#pragma once

#include <tos/compiler.hpp>

namespace tos::xtensa {
ALWAYS_INLINE
void set_stack_ptr(char* ptr) {
    __asm__ __volatile__("mov sp, %0" : : "r"(ptr) : "memory");
}

ALWAYS_INLINE
void* get_stack_ptr() {
    void* sp;
    __asm__ __volatile__("mov %0, a1;" : "=r"(sp)::);
    return sp;
}
} // namespace tos::xtensa