#pragma once

#include <type_traits>

namespace tos::meta {
template <class T> struct identity{
    using type = T;
};
template <class...> struct list {};
template<class T>
using clean_t = std::remove_const_t<std::remove_reference_t<T>>;

template<class T>
struct function_traits;

template<class RetT, class... ArgTs>
struct function_traits<RetT (*)(ArgTs...)> {
    using ret_t = RetT;
    using arg_ts = list<clean_t<ArgTs>...>;
    static constexpr auto arg_len = sizeof...(ArgTs);
};

template<class RetT, class... ArgTs>
struct function_traits<RetT (&)(ArgTs...)> {
    using ret_t = RetT;
    using arg_ts = list<clean_t<ArgTs>...>;
    static constexpr auto arg_len = sizeof...(ArgTs);
};

template<class RetT, class ClassT, class... ArgTs>
struct function_traits<RetT (ClassT::*)(ArgTs...)> {
    using ret_t = RetT;
    using class_t = ClassT;
    using arg_ts = list<clean_t<ArgTs>...>;
    static constexpr auto arg_len = sizeof...(ArgTs);
    static constexpr bool is_const = false;
};

template<class RetT, class ClassT, class... ArgTs>
struct function_traits<RetT (ClassT::*)(ArgTs...) const> {
    using ret_t = RetT;
    using class_t = ClassT;
    using arg_ts = list<clean_t<ArgTs>...>;
    static constexpr auto arg_len = sizeof...(ArgTs);
    static constexpr bool is_const = true;
};

template<class RetT, class ClassT, class... ArgTs>
struct function_traits<RetT (ClassT::*)(ArgTs...) noexcept> {
    using ret_t = RetT;
    using class_t = ClassT;
    using arg_ts = list<clean_t<ArgTs>...>;
    static constexpr auto arg_len = sizeof...(ArgTs);
    static constexpr bool is_const = true;
};
}