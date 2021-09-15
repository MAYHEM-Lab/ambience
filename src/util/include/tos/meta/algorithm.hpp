#pragma once

#include <tuple>

namespace tos::meta {
template<template<class...> class TupleT, class FnT, std::size_t... Is, class... Ts>
constexpr auto
transform_impl(const TupleT<Ts...>& tuple, FnT&& fn, std::index_sequence<Is...>) {
    return std::make_tuple(fn(std::get<Is>(tuple))...);
}

template<template<class...> class TupleT, class FnT, class... Ts>
constexpr auto transform(const TupleT<Ts...>& tuple, FnT&& fn) {
    return transform_impl(
        tuple, std::forward<FnT>(fn), std::make_index_sequence<sizeof...(Ts)>{});
}

template<template<class...> class TupleT,
         class T,
         class BinopT,
         std::size_t... Is,
         class... Ts>
constexpr T accumulate_impl(const TupleT<Ts...>& tuple,
                            T init,
                            BinopT&& binop,
                            std::index_sequence<Is...>) {
    return ((init = binop(std::move(init), std::get<Is>(tuple))), ...), init;
}

template<template<class...> class TupleT,
         class T,
         class... Ts,
         class BinopT = std::plus<>>
constexpr T accumulate(const TupleT<Ts...>& tuple, T init, BinopT&& binop = {}) {
    return accumulate_impl(tuple,
                           std::move(init),
                           std::forward<BinopT>(binop),
                           std::make_index_sequence<sizeof...(Ts)>{});
}

static_assert(accumulate(std::make_tuple(1, 2, 3), 0) == 6);
static_assert(accumulate(std::make_tuple(), 0) == 0);
} // namespace tos::meta