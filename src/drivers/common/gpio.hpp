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
        using high_t = std::true_type;
        using low_t = std::false_type;

        static constexpr high_t high{};
        static constexpr low_t low{};
    }

    namespace devs
    {
        using gpio_t = dev<struct _gpio_t, 0>;
        static constexpr gpio_t gpio{};
    }
}
