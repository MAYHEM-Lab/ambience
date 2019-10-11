//
// Created by Mehmet Fatih BAKIR on 01/06/2018.
//

#pragma once

#include <stm32_hal/hal.hpp>
#include <string.h>

extern "C" {
inline void __attribute__((always_inline)) tos_set_stack_ptr(char* ptr) {
    __asm__ __volatile__("mov sp, %0" : : "r"(ptr) : "memory");
}

inline void* __attribute__((always_inline)) tos_get_stack_ptr() {
    void* sp;
    __asm__ __volatile__("mov %0, sp" : "=r"(sp) : : "memory");
    return sp;
}

inline void tos_enable_interrupts() __attribute__((always_inline));
inline void tos_enable_interrupts() {
    __enable_irq();
}

inline void tos_disable_interrupts() __attribute__((always_inline));
inline void tos_disable_interrupts() {
    __disable_irq();
}
}

namespace tos {
namespace stm32 {
extern uint32_t apb1_clock;
extern uint32_t ahb_clock;
} // namespace stm32
} // namespace tos