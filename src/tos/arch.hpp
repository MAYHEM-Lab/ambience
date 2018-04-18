//
// Created by fatih on 4/16/18.
//

#pragma once

extern "C"
{
void tos_set_stack_ptr(void* ptr);
void tos_power_down();
void* tos_stack_alloc(size_t size);
void tos_stack_free(void*);
void tos_shutdown();
void tos_enable_interrupts();
void tos_disable_interrupts();
}

namespace tos
{
    inline void enable_interrupts() { tos_enable_interrupts(); }
    inline void disable_interrupts() { tos_disable_interrupts(); }
}