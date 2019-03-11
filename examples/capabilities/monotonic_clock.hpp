//
// Created by fatih on 9/22/18.
//

#pragma once

#include <chrono>
extern "C"
{
#include <user_interface.h>
}

namespace tos
{
    struct high_resolution_clock
    {
    public:
        using rep = uint32_t;
        using period = std::micro;
        using duration = std::chrono::duration<rep, period>;
        using time_point = std::chrono::time_point<high_resolution_clock>;

        static const bool is_steady = true;

        static time_point now()
        {
            return time_point{ duration(system_get_time()) };
        }
    };
} // namespace tos