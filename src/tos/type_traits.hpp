//
// Created by Mehmet Fatih BAKIR on 25/03/2018.
//
#pragma once

namespace tos
{
    template <class T>
    struct remove_reference { using type = T; };
    template <class T>
    struct remove_reference<T&> { using type = T; };
    template <class T>
    struct remove_reference<T&&> { using type = T; };
    template <class T>
    using remove_reference_t = typename remove_reference<T>::type;

    template <class T>
    struct is_lvalue_reference
    {
        static constexpr bool value = false;
        constexpr explicit operator bool() const
        {
            return value;
        }
    };

    template <class T>
    struct is_lvalue_reference<T&>
    {
        static constexpr bool value = true;
    };

}

