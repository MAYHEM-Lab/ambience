//
// Created by Mehmet Fatih BAKIR on 25/03/2018.
//

#pragma once

#include <stddef.h>
#include <tos/span.hpp>

namespace tos
{
    struct char_istream
    {
        virtual span<char> read(span<char>) = 0;
        virtual ~char_istream() = default;
    };

    struct char_ostream
    {
        virtual int write(span<const char>) = 0;
        virtual ~char_ostream() = default;
    };

    struct char_iostream :
            public char_istream,
            public char_ostream
    {
    };
}
