//
// Created by fatih on 6/12/18.
//

#pragma once

#include <stdint.h>

namespace tos
{
    struct microseconds
    {
        uint64_t val;
    };

    struct milliseconds
    {
        uint64_t val;

        explicit operator microseconds() const {
            return { val * 1000 };
        }
    };

    struct seconds
    {
        uint64_t val;
        explicit operator milliseconds() const {
            return { val * 1000 };
        }
    };

    namespace chrono_literals
    {
        constexpr milliseconds operator""_ms(unsigned long long arg)
        {
            return { arg };
        }
    }
}
