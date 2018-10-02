//
// Created by Mehmet Fatih BAKIR on 29/04/2018.
//

#pragma once

#include <drivers/common/tty.hpp>
#include <drivers/common/usart.hpp>
#include <tos/span.hpp>
#undef putc

namespace tos
{
    namespace x86
    {
        class stdio
        {
        public:
            static void putc(char);
            static void write(span<const char> buf);
            static int read(span<char> buf);
        };
    }

    template <class T>
    inline x86::stdio* open_impl(devs::usart_t<0>, T)
    {
        return nullptr;
    }

    inline x86::stdio* open_impl(devs::tty_t<0>)
    {
        return nullptr;
    }
}
