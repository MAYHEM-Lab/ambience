#pragma once

#include <tos/meta_object/dictionary.hpp>
#include <tos/meta_object/key_value.hpp>
#include <tos/print.hpp>

namespace tos::meta_object {
template<class Stream>
constexpr void print(Stream& str, const KeyValue auto& kv) {
    using tos::print;
    print(str, kv.key(), ':', kv.value());
}

template<class StreamT, KeyValue... KVs>
constexpr void print(StreamT& str, const dictionary<KVs...>& d) {
    print(str, '{');
    (print(str, static_cast<const KVs&>(d)), ...);
    print(str, '}');
}
} // namespace tos::meta_object