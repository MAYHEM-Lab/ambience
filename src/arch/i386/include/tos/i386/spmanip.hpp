#pragma once

#include <tos/compiler.hpp>

#undef i386

namespace tos::i386 {
ALWAYS_INLINE
void set_stack_ptr(char* ptr) {
    __asm__ __volatile__("movq %0, %%esp" : : "r"(ptr) : "memory");
}

ALWAYS_INLINE
void* get_stack_ptr() {
    void* sp;
    __asm__ __volatile__("movq %%esp, %0" : "=r"(sp) : : "memory");
    return sp;
}
} // namespace tos::i386
