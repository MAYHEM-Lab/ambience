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
    }(std::make_index_sequence<arity>{});
}
} // namespace tos::meta
