#pragma once

#include <string_view>

namespace lidl {
template <class T>
struct enum_traits;

template <class T>
constexpr bool is_enum_v = std::is_enum_v<T>;

template <class T, std::enable_if_t<is_enum_v<T>, void>* = nullptr>
constexpr std::string_view nameof(T t) {
    return enum_traits<T>::names[static_cast<int>(t)];
}
}