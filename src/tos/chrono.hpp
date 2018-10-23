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

        microseconds() = delete;
    };

    struct milliseconds
    {
        uint64_t val;

        milliseconds() = delete;

        operator microseconds() const {
            return { val * 1000 };
        }
    };

    struct seconds
    {
        uint64_t val;

        seconds() = delete;

        operator milliseconds() const {
            return { val * 1000 };
        }
    };

    namespace chrono_literals
    {
        constexpr milliseconds operator""_ms(unsigned long long arg)
        {
            return milliseconds{ arg };
        }

        constexpr microseconds operator""_us(unsigned long long arg)
        {
            return microseconds{ arg };
        }

        constexpr seconds operator""_s(unsigned long long arg)
        {
            return seconds{ arg };
        }
    }
}
