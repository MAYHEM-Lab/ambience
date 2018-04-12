//
// Created by Mehmet Fatih BAKIR on 25/03/2018.
//
#pragma once

#include <tos/type_traits.hpp>

namespace tos
{
    template <class T>
    T&& forward(remove_reference_t<T>& t) noexcept
    {
        return static_cast<T&&>(t);
    }

    template <class T>
    T&& forward(remove_reference_t<T>&& t) noexcept
    {
        static_assert(!is_lvalue_reference<T>{},
                "Can not forward an rvalue as an lvalue.");
        return static_cast<T&&>(t);
    }

    template <class T>
    remove_reference_t<T>&& move(T&& t)
    {
        return static_cast<remove_reference_t<T>&&>(t);
    }
}

