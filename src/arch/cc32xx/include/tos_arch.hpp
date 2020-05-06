//
// Created by fatih on 10/31/19.
//

#pragma once

#include <tos/compiler.hpp>

extern "C" {
ALWAYS_INLINE
void tos_set_stack_ptr(char* ptr) {
    __asm__ __volatile__("mov sp, %0" : : "r"(ptr) : "memory");
}

ALWAYS_INLINE
void* tos_get_stack_ptr() {
    void* sp;
    __asm__ __volatile__("mov %0, sp" : "=r"(sp) : : "memory");
    return sp;
}
}
