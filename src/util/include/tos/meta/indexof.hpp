#pragma once

#include <tuple>
#include <utility>

namespace tos::meta {
template<auto T, const auto& Tuple>
constexpr int tuple_index_of() {
    constexpr auto arity =
        std::tuple_size_v<std::remove_const_t<std::remove_reference_t<decltype(Tuple)>>>;
    return []<std::size_t... Is>(std::index_sequence<Is...>) {
        int res = -1;
        ((res = std::get<Is>(Tuple) == T ? Is : res), ...);
        return res;
    }
    (std::make_index_sequence<arity>{});
}

namespace detail {
template<auto needle, auto... haystack, size_t... Is>
constexpr int32_t do_index_of(std::index_sequence<Is...>) {
    int32_t res = 0;
    ((haystack == needle && (res = (Is + 1))) || ...);
    return res - 1;
}
} // namespace detail

template<auto needle, auto... haystack>
constexpr int32_t index_of() {
    return detail::do_index_of<needle, haystack...>(
        std::make_index_sequence<sizeof...(haystack)>{});
}
} // namespace tos::meta
