//
// Created by fatih on 4/19/18.
//

#pragma once

#include <type_traits>

template <class...> struct list {};

template <class...> struct tail;
template <class T, class... Ts> struct tail<list<T, Ts...>> { using type = list<Ts...>; };
template <class... Ts> using tail_t = typename tail<Ts...>::type;

template <class T>
using clean_t = std::remove_const_t<std::remove_reference_t<T>>;

template<class T>
struct function_traits;

template<class RetT, class... ArgTs>
struct function_traits<RetT(*)(ArgTs...)>
{
    using ret_t = RetT;
    using arg_ts = list<clean_t<ArgTs>...>;
    static inline constexpr auto arg_len = sizeof...(ArgTs);
};

template<class RetT, class... ArgTs>
struct function_traits<RetT(&)(ArgTs...)>
{
    using ret_t = RetT;
    using arg_ts = list<clean_t<ArgTs>...>;
    static inline constexpr auto arg_len = sizeof...(ArgTs);
};

template <auto, auto> struct ins {

};

template <auto opcode, auto X>
constexpr auto get_by_code(const ins<opcode, X>&)
{
    return X;
}

template <auto X, auto opcode>
constexpr auto get_by_fun(const ins<opcode, X>&)
{
    return opcode;
}