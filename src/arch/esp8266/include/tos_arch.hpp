//
// Created by fatih on 4/26/18.
//

#pragma once

#include "tos_platform.hpp"

extern "C"
{
inline void __attribute__((always_inline)) tos_set_stack_ptr(char* ptr)
{
    __asm__ __volatile__("mov sp, %0" : : "r"(ptr) : "memory");
}

inline void* __attribute__((always_inline)) read_sp()
{
   void* sp;
   __asm__ __volatile__("mov %0, a1;" : "=r"(sp) ::);
   return sp;
}
}

namespace tos
{
    namespace esp82
    {
        static constexpr auto main_task_prio = 0;
    }
}
