//
// Created by Mehmet Fatih BAKIR on 29/04/2018.
//

#pragma once

#include <tos/devices.hpp>

namespace tos
{
    namespace devs
    {
        template <int N> using tty_t = dev<struct _tty_t, N>;
        template <int N> static constexpr tty_t<N> tty{};
    }
}
