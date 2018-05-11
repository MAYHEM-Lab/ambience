//
// Created by Mehmet Fatih BAKIR on 29/03/2018.
//

#pragma once

#include <tos/devices.hpp>
#include <stdint.h>

namespace tos
{
    struct pin_t;
    namespace tos_literals
    {
        pin_t operator""_pin(unsigned long long pin);
    }

    namespace devs
    {
        using gpio_t = dev<struct _gpio_t, 0>;
        static constexpr gpio_t gpio{};
    }
}
