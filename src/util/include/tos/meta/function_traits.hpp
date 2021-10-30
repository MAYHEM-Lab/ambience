#pragma once

#include <type_traits>
#include <tos/meta/types.hpp>

namespace tos::meta {
template<class T>
struct function_traits;

template<class RetT, class... ArgTs>
struct function_traits<RetT (*)(ArgTs...)> {
    using ret_t = RetT;
    using arg_ts = list<ArgTs...>;
    static constexpr auto arg_len = sizeof...(ArgTs);
};

template<class RetT, class... ArgTs>
struct function_traits<RetT (&)(ArgTs...)> {
    using ret_t = RetT;
    using arg_ts = list<ArgTs...>;
    static constexpr auto arg_len = sizeof...(ArgTs);
};

template<class RetT, class ClassT, class... ArgTs>
struct function_traits<RetT (ClassT::*)(ArgTs...)> {
    using ret_t = RetT;
    using class_t = ClassT;
    using arg_ts = list<ArgTs...>;
    static constexpr auto arg_len = sizeof...(ArgTs);
    static constexpr bool is_const = false;
};

template<class RetT, class ClassT, class... ArgTs>
struct function_traits<RetT (ClassT::*)(ArgTs...) const> {
    using ret_t = RetT;
    using class_t = ClassT;
    using arg_ts = list<ArgTs...>;
    static constexpr auto arg_len = sizeof...(ArgTs);
    static constexpr bool is_const = true;
};

template<class RetT, class ClassT, class... ArgTs>
struct function_traits<RetT (ClassT::*)(ArgTs...) noexcept> {
    using ret_t = RetT;
    using class_t = ClassT;
    using arg_ts = list<ArgTs...>;
    static constexpr auto arg_len = sizeof...(ArgTs);
    static constexpr bool is_const = true;
};
}