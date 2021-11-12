#pragma once

#include <tos/fixed_string.hpp>
#include <tos/meta_object/key.hpp>
#include <tos/meta_object/key_value.hpp>
#include <tos/meta_object/meta.hpp>

namespace tos::meta_object {
template<KeyValue... KVs>
struct dictionary : KVs... {
    template<KeyValue... KVArgs>
    constexpr dictionary(KVArgs&&... init) {
        (assign(init), ...);
    }

    template<KeyValue... KVArgs>
    constexpr dictionary(const dictionary<KVArgs...>& rhs) {
        (assign(static_cast<const KVArgs&>(rhs)), ...);
    }

    template<fixed_string Key>
    constexpr decltype(auto) operator[](const key_t<Key>&) {
        using tos::meta_object::get;
        return get<Key>(*this);
    }

    template<fixed_string Key>
    constexpr decltype(auto) operator[](const key_t<Key>&) const {
        using tos::meta_object::get;
        return get<Key>(*this);
    }

    template<fixed_string Key>
    static constexpr bool has_key() {
        return meta::has_key<Key, dictionary>::value;
    }

private:
    template<fixed_string Key, class T>
    constexpr void assign(const kv<Key, T>& arg) {
        if constexpr (has_key<Key>()) {
            assign<Key>(*this, arg);
        }
    }

    template<fixed_string Key, class T, class U>
    constexpr void assign(kv<Key, T>& x, const kv<Key, U> arg) {
        x.m_val = arg.value();
    }
};
} // namespace tos::meta_object