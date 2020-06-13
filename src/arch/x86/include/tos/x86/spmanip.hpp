#pragma once

#include <tos/compiler.hpp>

namespace tos::x86 {
ALWAYS_INLINE
void set_stack_ptr(char* ptr) {
    __asm__ __volatile__("movq %0, %%rsp" : : "r"(ptr) : "memory");
}

ALWAYS_INLINE
void* get_stack_ptr() {
    void* sp;
    __asm__ __volatile__("movq %%rsp, %0" : "=r"(sp) : : "memory");
    return sp;
}
} // namespace tos::x86
