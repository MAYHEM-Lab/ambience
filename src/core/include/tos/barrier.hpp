//
// Created by fatih on 10/5/18.
//

#pragma once

namespace tos
{
namespace detail
{
    inline void memory_barrier_enter()
    {
        asm volatile ("" ::: "memory");
    }

    inline void memory_barrier_exit()
    {
        asm volatile ("" ::: "memory");
    }
}
}