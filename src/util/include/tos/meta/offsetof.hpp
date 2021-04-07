#pragma once

#include <cstddef>
#include <cstdint>
#include <type_traits>
#include <utility>

namespace tos::meta {
namespace detail {
template<typename T>
struct declval_helper {
    static inline T value;
};

template<typename T, typename Z, Z T::*MPtr>
struct offset_helper {
    using TV = declval_helper<T>;
    char for_sizeof[(char*)&(TV::value.*MPtr) - (char*)&TV::value];
};

template<typename T, typename Z, Z T::*MPtr>
constexpr int offset_of() {
    return sizeof(detail::offset_helper<T, Z, MPtr>::for_sizeof);
}

template<class T1, class T2>
auto class_type(T1 T2::*) -> T2;

template<class T1, class T2>
auto return_type(T1 T2::*) -> T1;
} // namespace detail

template<auto PtrToMem>
constexpr int offset_of() {
    using class_t = decltype(detail::class_type(PtrToMem));
    using ret_t = decltype(detail::return_type(PtrToMem));
    return detail::offset_of<class_t, ret_t, PtrToMem>();
}
} // namespace tos::meta