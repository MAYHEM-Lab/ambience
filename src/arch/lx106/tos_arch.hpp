//
// Created by fatih on 4/26/18.
//

#pragma once

#include "tos_platform.hpp"

extern "C"
{
#include <user_interface.h>
void tos_set_stack_ptr(void*);

inline void* read_sp()
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
        static constexpr auto main_task_prio = USER_TASK_PRIO_0;
    }
}
