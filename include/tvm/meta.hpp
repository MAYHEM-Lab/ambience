//
// Created by fatih on 4/19/18.
//
/**
 * @file This file contains the template metaprogramming utility
 * templates and functions
 */

#pragma once

#ifdef TOS
#include <tos/tuple.hpp>
#include <tos/type_traits.hpp>
namespace std
{
    using namespace tos::std;
}
#else
#include <tuple>
    using std::tuple;
    using std::index_sequence;
    using std::integral_constant;
    using std::remove_const_t;
    using std::get;
    using std::remove_reference_t;
    using std::forward;
    using std::tuple_size;
    using std::make_index_sequence;
#endif

namespace tvm
{
    /**
     * This class template represents types as values at compile time
     * @tparam T type to represent
     */
    template <class T> struct identity{
        using type = T;
    };

    /**
     * This class template represents a list of types
     * @tparam ...
     */
    template <class...> struct list {};

    template <class...> struct tail;

    /**
     * Pops the first type from a type list and returns the tail
     * as a new list
     * @param list<T, Ts...> input list
     * @returns list<Ts...>
     */
    template <class T, class... Ts> struct tail<list<T, Ts...>> { using type = list<Ts...>; };
    template <class... Ts> using tail_t = typename tail<Ts...>::type;

    template <class... Ts>
    constexpr auto len(list<Ts...>)
    {
        return sizeof...(Ts);
    }

    template <int, class...> struct type_at;

    template <class Front, class... Ts>
    struct type_at<0, list<Front, Ts...>>
    {
        using type = Front;
    };

    /**
     * Gets the type at the i'th index in the given list
     *
     * Use type_at_t directly instead of this
     *
     * @tparam i index
     * @tparam list<Ts..> input list
     */
    template <int i, class... Ts>
    struct type_at<i, list<Ts...>>
    {
        using type = typename type_at <i - 1, tail_t<list<Ts...>>>::type;
    };

    /**
     * Gets the type at the i'th index in the given list
     *
     * @tparam i index
     * @tparam list<Ts..> input list
     */
    template<int i, class... Ts>
    using type_at_t = typename type_at<i, Ts...>::type;

    template <class T>
    using clean_t = std::remove_const_t<std::remove_reference_t<T>>;

    namespace detail
    {
        template<class T>
        struct function_traits;

        template<class RetT, class... ArgTs>
        struct function_traits<RetT(*)(ArgTs...)>
        {
            using ret_t = RetT;
            using arg_ts = list<clean_t<ArgTs>...>;
            static constexpr auto arg_len = sizeof...(ArgTs);
        };

        template<class RetT, class... ArgTs>
        struct function_traits<RetT(&)(ArgTs...)>
        {
            using ret_t = RetT;
            using arg_ts = list<clean_t<ArgTs>...>;
            static constexpr auto arg_len = sizeof...(ArgTs);
        };

        template <class RetT, class ClassT, class... ArgTs>
        struct function_traits<RetT (ClassT::*)(ArgTs...)>
        {
            using ret_t = RetT;
            using arg_ts = list<clean_t<ArgTs>...>;
            static constexpr auto arg_len = sizeof...(ArgTs);
        };
    }
    template <class T>
    struct functor_traits
            : detail::function_traits<decltype(&T::operator())> {};

    template <class RetT, class...ArgTs>
    struct functor_traits<RetT(&)(ArgTs...)>
            : detail::function_traits<RetT(&)(ArgTs...)> {};

    template <class RetT, class...ArgTs>
    struct functor_traits<RetT(*)(ArgTs...)>
            : detail::function_traits<RetT(*)(ArgTs...)> {};

    template <uint8_t, class> struct ins {};

    template <uint8_t opcode, class X>
    constexpr auto get_by_code(const ins<opcode, X>&)
    {
        return identity<X>{};
    }

    template <class X, uint8_t opcode>
    constexpr auto get_by_fun(const ins<opcode, X>&)
    {
        return opcode;
    }

    template <class... Instrs>
    struct isa_map : Instrs...
    {};

    template <class... Instrs>
    struct isa_map<list<Instrs...>> : Instrs...
    {};

    namespace detail {
        template <class F, class AddT, class Tuple, size_t... I>
        constexpr decltype(auto) apply_impl(F&& f, AddT&& a, Tuple&& t, std::index_sequence<I...>)
        {
            return std::forward<F>(f)(std::forward<AddT>(a), std::get<I>(std::forward<Tuple>(t))...);
        }
    }

    template< class T >
    constexpr size_t tuple_size_v = std::tuple_size<T>::value;

    template <class F, class AddT, class Tuple>
    constexpr decltype(auto) apply(F&& f, AddT&& a, Tuple&& t)
    {
        return detail::apply_impl(
                std::forward<F>(f), std::forward<AddT>(a), std::forward<Tuple>(t),
                std::make_index_sequence<tuple_size_v<std::remove_reference_t<Tuple>>>());
    }
}