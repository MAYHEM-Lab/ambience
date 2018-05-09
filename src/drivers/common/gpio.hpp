//
// Created by Mehmet Fatih BAKIR on 29/03/2018.
//

#pragma once

#include <stdint.h>

namespace tos
{
    struct pin_t;
    namespace tos_literals
    {
        pin_t operator""_pin(unsigned long long pin);
    }
}
