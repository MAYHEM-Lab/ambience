#pragma once

#include <tos/meta/types.hpp>

namespace tos::meta {
template<class A, class B>
struct concat;

template<auto... Ts, auto... Us>
struct concat<values<Ts...>, values<Us...>> {
    using type = values<Ts..., Us...>;
};

template<class... Ts, class... Us>
struct concat<list<Ts...>, list<Us...>> {
    using type = list<Ts..., Us...>;
};

template<class... Ts>
using concat_t = typename concat<Ts...>::type;
} // namespace tos::meta