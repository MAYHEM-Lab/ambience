//
// Created by Mehmet Fatih BAKIR on 23/10/2018.
//

#pragma once

#include <stddef.h>
#include <initializer_list>

namespace tos
{
    template <class T, size_t Sz>
    class array
    {
    public:
        constexpr array() : m_arr{} {}

        //constexpr array(std::initializer_list<T> il) : m_arr{il} {}

    private:
        T m_arr[Sz];
    };
}
