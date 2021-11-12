#pragma once

#include <tos/meta_object/key_value.hpp>

namespace tos::meta_object {
template<fixed_string Key>
struct key_t {
    template<class T>
    constexpr kv<Key, T> operator=(const T& t) const {
        return {t};
    }
};

template<fixed_string Key>
constexpr inline auto key = key_t<Key>{};
} // namespace tos::meta_object