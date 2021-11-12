#pragma once

#include <tos/fixed_string.hpp>
#include <tos/print.hpp>
#include <type_traits>

namespace tos::meta_object {
template<fixed_string Key, class ValueT>
struct kv {
    static constexpr fixed_string key_v = Key;

    constexpr std::string_view key() const {
        return std::string_view(Key);
    }

    constexpr const auto& value() const {
        return m_val;
    }

    ValueT m_val;
};

template<fixed_string Key, class T>
constexpr T& get(kv<Key, T>& el) {
    return el.m_val;
}

template<fixed_string Key, class T>
constexpr const T& get(const kv<Key, T>& el) {
    return el.m_val;
}
} // namespace tos::meta_object