//
// Created by fatih on 4/18/18.
//

#pragma once

#include <tos/devices.hpp>

namespace tos
{
    namespace devs {
        template<int N> using timer_t = dev<struct _timer_t, N>;
        template<int N> static constexpr timer_t<N> timer{};
    }
}