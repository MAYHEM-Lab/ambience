#pragma once

#include <tos/meta_object/key_value.hpp>
#include <tos/type_traits.hpp>

namespace tos::meta_object {
template<fixed_string Key, class ValueT>
struct kv;

namespace meta {
template<class T>
struct is_kv : std::false_type {};

template<fixed_string Key, class ValueT>
struct is_kv<kv<Key, ValueT>> : std::true_type {};
} // namespace meta

template<class T>
concept KeyValue = meta::is_kv<T>::value;

template<KeyValue... KVs>
struct dictionary;

namespace meta {
template<fixed_string Key, class Dictionary>
struct has_key;

template<fixed_string Key, fixed_string... Keys, class... Ts>
struct has_key<Key, dictionary<kv<Keys, Ts>...>> {
    static constexpr bool value = (is_same<Keys, Key>::value || ...);
};
} // namespace meta
} // namespace tos::meta_object