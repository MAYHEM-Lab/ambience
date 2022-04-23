#pragma once

#include "tos/span.hpp"
#include <lidlrt/concepts.hpp>
#include <lidlrt/string.hpp>
#include <lidlrt/vector.hpp>
#include <type_traits>
#include <utility>

namespace lidl::meta::detail {
inline tos::span<const uint8_t> bounding_span(tos::span<const uint8_t> a) {
    return a;
}

inline tos::span<const uint8_t> bounding_span(tos::span<const uint8_t> a,
                                              tos::span<const uint8_t> b) {
    auto min = std::min(a.begin(), b.begin());
    auto max = std::max(a.end(), b.end());
    return {min, max};
}

template<class... T>
tos::span<const uint8_t> bounding_span(tos::span<const uint8_t> a, T&&... ts) {
    return bounding_span(a, bounding_span(ts...));
}

inline tos::span<const uint8_t> find_extent(tos::span<const uint8_t> a) {
    return a;
}

inline tos::span<const uint8_t> find_extent(std::string_view a) {
    return tos::span<const uint8_t>(reinterpret_cast<const uint8_t*>(a.data()), a.size());
}

inline tos::span<const uint8_t> find_extent(const size_t& sz) {
    return tos::raw_cast(tos::monospan(sz));
}

inline tos::span<const uint8_t> find_extent(const unsigned long long& sz) {
    return tos::raw_cast(tos::monospan(sz));
}

template<Struct ObjT>
tos::span<const uint8_t> find_extent(const ObjT& obj);

tos::span<const uint8_t> find_extent(const Union auto& obj);

template<class T>
tos::span<const uint8_t> find_extent(const lidl::vector<T>& obj);
tos::span<const uint8_t> find_extent(const lidl::string& str);

template<class T>
requires std::is_pod_v<T> tos::span<const uint8_t> find_extent(const T& t) {
    return tos::raw_cast<const uint8_t>(tos::monospan(t));
}

template<class T>
tos::span<const uint8_t> find_extent(const lidl::vector<T>& vec) {
    auto buf = tos::raw_cast<const uint8_t>(vec.span());
    return bounding_span(tos::raw_cast<const uint8_t>(tos::monospan(vec)), buf);
}

inline tos::span<const uint8_t> find_extent(const lidl::string& str) {
    auto buf = tos::span<const uint8_t>{
        reinterpret_cast<const uint8_t*>(str.string_view().data()),
        str.string_view().size()};
    return bounding_span(tos::raw_cast<const uint8_t>(tos::monospan(str)), buf);
}

template<class ObjT, class... Members, std::size_t... Is>
tos::span<const uint8_t> aggregate_extent(const ObjT& obj,
                                          std::index_sequence<Is...>,
                                          const std::tuple<Members...>& members) {
    return bounding_span(find_extent((obj.*std::get<Is>(members).const_function)())...);
}

template<Struct ObjT>
tos::span<const uint8_t> find_extent(const ObjT& obj) {
    using traits = struct_traits<ObjT>;
    using members_tuple_t =
        std::remove_const_t<std::remove_reference_t<decltype(traits::members)>>;
    static constexpr size_t members_size = std::tuple_size_v<members_tuple_t>;
    return bounding_span(
        tos::raw_cast<const uint8_t>(tos::monospan(obj)),
        aggregate_extent(obj, std::make_index_sequence<members_size>{}, traits::members));
}

tos::span<const uint8_t> find_extent(const Union auto& obj) {
    auto active = visit([](auto& member) { return find_extent(member); }, obj);
    return bounding_span(tos::raw_cast<const uint8_t>(tos::monospan(obj)), active);
}

template<class ObjT>
std::pair<tos::span<const uint8_t>, ptrdiff_t> find_extent_and_position(const ObjT& obj) {
    auto __extent = find_extent(obj);
    auto __ptr = reinterpret_cast<const uint8_t*>(&obj);
    return {__extent, __ptr - __extent.begin()};
}
} // namespace lidl::meta::detail