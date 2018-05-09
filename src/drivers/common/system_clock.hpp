//
// Created by fatih on 5/8/18.
//

#pragma once

#include <stdint.h>

namespace tos
{
    struct milliseconds
    {
        uint64_t val;
    };

    template <class T>
    class clock
    {
    public:
        clock(T& t);

        milliseconds now(); // return milliseconds

    private:
        T* m_timer;
    };
}