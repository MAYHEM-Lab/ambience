//
// Created by fatih on 4/19/18.
//

#pragma once

#include <type_traits>
#include <tuple>

namespace tvm
{
    template <class T> struct ctype{
        using type = T;
    };
    template <class...> struct list {};

    template <class...> struct tail;
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

    template <int i, class... Ts>
    struct type_at<i, list<Ts...>>
    {
        using type = typename type_at <i - 1, tail_t<list<Ts...>>>::type;
    };

    template<int i, class... Ts>
    using type_at_t = typename type_at<i, Ts...>::type;

    template <class T>
    using clean_t = std::remove_const_t<std::remove_reference_t<T>>;

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

    template <class T>
    struct functor_traits
            : function_traits<decltype(&T::operator())> {};

    template <class RetT, class...ArgTs>
    struct functor_traits<RetT(&)(ArgTs...)>
            : function_traits<RetT(&)(ArgTs...)> {};

    template <class RetT, class...ArgTs>
    struct functor_traits<RetT(*)(ArgTs...)>
            : function_traits<RetT(*)(ArgTs...)> {};

    template <uint8_t, class> struct ins {
    };

    template <uint8_t opcode, class X>
    constexpr auto get_by_code(const ins<opcode, X>&)
    {
        return ctype<X>{};
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
        template <class F, class AddT, class Tuple, std::size_t... I>
        constexpr decltype(auto) apply_impl(F&& f, AddT&& a, Tuple&& t, std::index_sequence<I...>)
        {
            return std::forward<F>(f)(std::forward<AddT>(a), std::get<I>(std::forward<Tuple>(t))...);
        }
    }  // namespace detail

    template< class T >
    constexpr std::size_t tuple_size_v = std::tuple_size<T>::value;

    template <class F, class AddT, class Tuple>
    constexpr decltype(auto) apply(F&& f, AddT&& a, Tuple&& t)
    {
        return detail::apply_impl(
                std::forward<F>(f), std::forward<AddT>(a), std::forward<Tuple>(t),
                std::make_index_sequence<tuple_size_v<std::remove_reference_t<Tuple>>>{});
    }
}