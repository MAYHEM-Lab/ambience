#pragma once

#include <tos/meta/concat.hpp>
#include <tos/meta/types.hpp>

namespace tos::meta {
template<int Count, class... Ts>
struct take;

template<int Count, class... Ts>
using take_t = typename take<Count, Ts...>::type;

template<class... Ts>
struct take<0, list<Ts...>> {
    using type = list<>;
    using rest = list<Ts...>;
};

template<auto... Ts>
struct take<0, values<Ts...>> {
    using type = values<>;
    using rest = values<Ts...>;
};

template<class A, class... Ts>
struct take<1, list<A, Ts...>> {
    using type = list<A>;
    using rest = list<Ts...>;
};

template<auto A, auto... Ts>
struct take<1, values<A, Ts...>> {
    using type = values<A>;
    using rest = values<Ts...>;
};

template<int Count, class A, class... Ts>
struct take<Count, list<A, Ts...>> {
    using type = concat_t<list<A>, take_t<Count - 1, list<Ts...>>>;
    using rest = typename take<Count - 1, list<Ts...>>::rest;
};

template<int Count, auto A, auto... Ts>
struct take<Count, values<A, Ts...>> {
    using type = concat_t<values<A>, take_t<Count - 1, values<Ts...>>>;
    using rest = typename take<Count - 1, values<Ts...>>::rest;
};
} // namespace tos::meta