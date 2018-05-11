//
// Created by Mehmet Fatih BAKIR on 25/03/2018.
//
#pragma once

#include <tos/type_traits.hpp>
#include <stdint.h>
#include <stddef.h>

namespace tos
{
    template <class T>
    constexpr T&& forward(remove_reference_t<T>& t) noexcept
    {
        return static_cast<T&&>(t);
    }

    template <class T>
    constexpr T&& forward(remove_reference_t<T>&& t) noexcept
    {
        static_assert(!is_lvalue_reference<T>{},
                "Can not forward an rvalue as an lvalue.");
        return static_cast<T&&>(t);
    }

    template <class T>
    constexpr remove_reference_t<T>&& move(T&& t)
    {
        return static_cast<remove_reference_t<T>&&>(t);
    }

    template<class T> using invoke_t = typename T::type;

    template<class T, T...> struct integer_sequence
    {
        using type = integer_sequence;
    };

    template<class S1, class S2> struct concat;

    template<class T, T... I1, T... I2>
    struct concat<integer_sequence<T, I1...>, integer_sequence<T, I2...>>
            : integer_sequence<T, I1..., (sizeof...(I1)+I2)...>{};

    template<class S1, class S2>
    using concat_t = invoke_t<concat<S1, S2>>;

    template<class T, intmax_t N> struct make_integer_sequence_t;
    template<class T, T N> using generate_sequence = invoke_t<make_integer_sequence_t<T, N>>;

    template<class T, intmax_t N>
    struct make_integer_sequence_t :
            concat_t<generate_sequence<T, N/2>, generate_sequence<T, N - N/2>>{};

    template<class T> struct make_integer_sequence_t<T, 0> : integer_sequence<T>{};
    template<class T> struct make_integer_sequence_t<T, 1> : integer_sequence<T, 0>{};

    template <class T, T N>
    auto make_integer_sequence() -> generate_sequence<T, N>
    {
        return {};
    };

    template <size_t... N>
    using index_sequence = integer_sequence<size_t, N...>;

    template <size_t N>
    auto make_index_sequence() -> generate_sequence<size_t, N>
    {
        return {};
    }

    template <class... Ts>
    struct tuple_size;

    template <class... Ts>
    class tuple;

    template <class... Ts>
    struct tuple_size<tuple<Ts...>>
    {
        static constexpr size_t value = sizeof...(Ts);
    };
}

