//
// Created by Mehmet Fatih BAKIR on 29/03/2018.
//

#pragma once

#include <tos/devices.hpp>
#include <stdint.h>

namespace tos
{
    namespace pin_mode
    {
        struct input_t{};
        struct output_t{};
        struct in_pullup_t{};
        struct in_pulldown_t{};

        static constexpr input_t in{};
        static constexpr output_t out{};
        static constexpr in_pullup_t in_pullup{};
        static constexpr in_pulldown_t in_pulldown{};
    }

    namespace digital
    {
        using high_t = true_type;
        using low_t = false_type;

        static constexpr true_type high{};
        static constexpr false_type low{};
    }

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
