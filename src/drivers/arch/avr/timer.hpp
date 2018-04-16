//
// Created by fatih on 4/16/18.
//

#pragma once

#include <stdint.h>

namespace tos
{
namespace avr
{
    class timer1
    {
    public:
        static void set_interval(uint16_t microsecs);
    };
}
}