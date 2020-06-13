#pragma once

#include <tos/compiler.hpp>

namespace tos::arm {
ALWAYS_INLINE
void set_stack_ptr(char* ptr) {
    __asm__ __volatile__("mov sp, %0" : : "r"(ptr) : "memory");
}

ALWAYS_INLINE
void* get_stack_ptr() {
    void* sp;
    __asm__ __volatile__("mov %0, sp" : "=r"(sp) : : "memory");
    return sp;
}
} // namespace tos::arm