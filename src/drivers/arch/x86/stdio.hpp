//
// Created by Mehmet Fatih BAKIR on 29/04/2018.
//

#pragma once

#include <drivers/common/tty.hpp>

namespace tos
{
    namespace x86
    {
        class stdio
        {
        public:
            static void putc(char);
            static void write(const char* data, int len);
            static int read(char* data, int len);
        };
    }

    inline x86::stdio* open_impl(devs::tty_t<0>)
    {
        return nullptr;
    }
}
