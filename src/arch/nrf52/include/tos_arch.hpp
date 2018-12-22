//
// Created by Mehmet Fatih BAKIR on 01/06/2018.
//

#pragma once

#include <nrf52.h>
#include <string.h>

extern "C"
{
void tos_set_stack_ptr(char* ptr) __attribute__((always_inline));
inline void tos_set_stack_ptr(char* ptr)
{
    ptr -= 32;
    char* sp;
    __asm__ __volatile__("mov %0, sp" : "=r"(sp) : : "memory");
    memcpy(ptr, sp, 32);
    __asm__ __volatile__("mov sp, %0" : : "r"(ptr) : "memory");
}

inline void tos_enable_interrupts() __attribute__((always_inline));
inline void tos_enable_interrupts()
{
    __enable_irq();
}

inline void tos_disable_interrupts() __attribute__((always_inline));
inline void tos_disable_interrupts()
{
    __disable_irq();
}
}
