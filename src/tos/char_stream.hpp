//
// Created by Mehmet Fatih BAKIR on 25/03/2018.
//

#pragma once

#include <stddef.h>

namespace tos
{
    struct char_istream
    {
        virtual int read(char* buf, size_t sz) = 0;
        virtual char getc() = 0;
        virtual ~char_istream() = default;
    };

    struct char_ostream
    {
        virtual int write(const char* buf, size_t sz) = 0;
        virtual void putc(char c) = 0;
        virtual ~char_ostream() = default;
    };

    struct char_iostream :
            public char_istream,
            public char_ostream
    {
    };
}
