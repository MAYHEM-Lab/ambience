//
// Created by fatih on 5/29/18.
//

#pragma once

#include <tos/interrupt.hpp>

namespace tos
{
    inline void kernel_init()
    {
        tos::enable_interrupts();
    }
}