//
// Created by fatih on 10/31/19.
//

#pragma once

extern "C" {
inline void __attribute__((always_inline)) tos_set_stack_ptr(char* ptr) {
    __asm__ __volatile__("mov sp, %0" : : "r"(ptr) : "memory");
}

inline void* __attribute__((always_inline)) tos_get_stack_ptr() {
    void* sp;
    __asm__ __volatile__("mov %0, sp" : "=r"(sp) : : "memory");
    return sp;
}
}
