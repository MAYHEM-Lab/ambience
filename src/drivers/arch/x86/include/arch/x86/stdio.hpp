//
// Created by Mehmet Fatih BAKIR on 29/04/2018.
//

#pragma once

#include <common/tty.hpp>
#include <common/usart.hpp>
#include <tos/span.hpp>
#include <common/driver_base.hpp>

#undef putc

namespace tos
{
    namespace x86
    {
        class stdio : public self_pointing<stdio>
        {
        public:
            static int write(span<const char> buf);
            static span<char> read(span<char> buf);
        };
    } // namespace x86

    template <class T>
    inline x86::stdio open_impl(devs::usart_t<0>, T)
    {
        return {};
    }

    inline x86::stdio open_impl(devs::tty_t<0>)
    {
        return {};
    }
} // namespace tos
