#pragma once

#include <lidlrt/string.hpp>
#include <lidlrt/vector.hpp>
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

inline tos::span<const uint8_t> find_extent(const size_t& sz) {
    return tos::raw_cast(tos::monospan(sz));
}

template<class ObjT>
tos::span<const uint8_t> find_extent(const ObjT& obj);

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
    return bounding_span(
        find_extent((obj.*std::get<Is>(members).const_function)())...);
}

template<class ObjT>
tos::span<const uint8_t> find_extent(const ObjT& obj) {
    using traits = struct_traits<ObjT>;
    using members_tuple_t = std::remove_const_t<std::remove_reference_t<decltype(traits::members)>>;
    static constexpr size_t members_size = std::tuple_size_v<members_tuple_t>;
    return bounding_span(tos::raw_cast<const uint8_t>(tos::monospan(obj)),
                         aggregate_extent(obj, std::make_index_sequence<members_size>{}, traits::members));
}

template<class ObjT>
std::pair<tos::span<const uint8_t>, ptrdiff_t> find_extent_and_position(const ObjT& obj) {
    auto __extent = find_extent(obj);
    auto __ptr    = reinterpret_cast<const uint8_t*>(&obj);
    return {__extent, __ptr - __extent.begin()};
}
} // namespace lidl::meta::detail