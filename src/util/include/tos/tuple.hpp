//
// Created by Mehmet Fatih BAKIR on 10/05/2018.
//

#pragma once

#include <stddef.h>
#include <tos/utility.hpp>

namespace tos
{
    template <class T, size_t ndx>
    struct tuple_elem
    {
        T t;
        explicit tuple_elem(T&& t) : t(t) {}
    };

    template <class SeqT, class... T>
    class tuple_impl;

    template <class... T, size_t... Is>
    class tuple_impl<index_sequence<Is...>, T...>
        : public tuple_elem<T, Is>...
    {
    protected:
        template <class... Us>
        tuple_impl(Us&&... args)
            : tuple_elem<T, Is>(forward<Us>(args))...
        {
            static_assert(sizeof...(Us) == sizeof...(T), "Mismatched size");
        }
    };

    template <class... Ts>
    class tuple : public tuple_impl<decltype(make_index_sequence<sizeof...(Ts)>()), Ts...>
    {
    public:
        template <class... Us>
        tuple(Us&&... args) : tuple_impl<decltype(make_index_sequence<sizeof...(Ts)>()), Ts...>(forward<Us>(args)...) {}
    };

template <size_t I, class T>
    T& get(tuple_elem<T, I>& elem)
    {
        return elem.t;
    };
}

