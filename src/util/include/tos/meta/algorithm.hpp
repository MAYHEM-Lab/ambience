#pragma once

#include <boost/hana/detail/variadic/foldl1.hpp>
#include <boost/hana/detail/variadic/foldr1.hpp>
#include <functional>
#include <tuple>
#include <utility>

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
constexpr auto accumulate_impl(const TupleT<Ts...>& tuple,
                               T init,
                               BinopT&& binop,
                               std::index_sequence<Is...>) {
    return boost::hana::detail::variadic::foldl(
        std::forward<BinopT>(binop), init, std::get<Is>(tuple)...);
    // return ((init = binop(std::move(init), std::get<Is>(tuple))), ...), init;
}

template<template<class...> class TupleT,
         class T,
         class BinopT,
         std::size_t... Is,
         class... Ts>
constexpr auto foldr_impl(const TupleT<Ts...>& tuple,
                          T init,
                          BinopT&& binop,
                          std::index_sequence<Is...>) {
    return boost::hana::detail::variadic::foldr(
        std::forward<BinopT>(binop), init, std::get<Is>(tuple)...);
    // return ((init = binop(std::move(init), std::get<Is>(tuple))), ...), init;
}

template<template<class...> class TupleT,
         class BinopT,
         class InitT,
         class SepT,
         std::size_t... Is,
         class... Ts>
constexpr auto join_impl(const TupleT<Ts...>& tuple,
                         const SepT& separator,
                         InitT init,
                         BinopT&& binop,
                         std::index_sequence<0, Is...> is) {
    auto wrap_binop = [&](auto&& left, auto&& right) {
        return binop(binop(left, separator), right);
    };

    return accumulate_impl(
        tuple, binop(init, std::get<0>(tuple)), wrap_binop, std::index_sequence<Is...>{});
}

template<template<class...> class TupleT,
         class T,
         class... Ts,
         class BinopT = std::plus<>>
constexpr auto accumulate(const TupleT<Ts...>& tuple, T init, BinopT&& binop = {}) {
    return accumulate_impl(tuple,
                           std::move(init),
                           std::forward<BinopT>(binop),
                           std::make_index_sequence<sizeof...(Ts)>{});
}
template<template<class...> class TupleT,
         class T,
         class... Ts,
         class BinopT = std::plus<>>
constexpr auto foldr(const TupleT<Ts...>& tuple, T init, BinopT&& binop = {}) {
    return foldr_impl(tuple,
                      std::move(init),
                      std::forward<BinopT>(binop),
                      std::make_index_sequence<sizeof...(Ts)>{});
}

template<template<class...> class TupleT, class T, class SepT, class BinopT = std::plus<>>
constexpr auto join(const TupleT<>& tuple, T init, const SepT& sep, BinopT&& binop = {}) {
    return std::move(init);
}

template<template<class...> class TupleT,
         class... Ts,
         class InitT,
         class SepT,
         class BinopT = std::plus<>>
constexpr auto
join(const TupleT<Ts...>& tuple, InitT init, const SepT& sep, BinopT&& binop = {}) {
    return join_impl(tuple,
                     sep,
                     std::move(init),
                     std::forward<BinopT>(binop),
                     std::make_index_sequence<sizeof...(Ts)>{});
}

static_assert(accumulate(std::make_tuple(1, 2, 3), 0) == 6);
static_assert(accumulate(std::make_tuple(), 0) == 0);
static_assert(join(std::make_tuple(1), 0, 1) == 1);
static_assert(join(std::make_tuple(1, 2, 3), 0, 1) == 8);
} // namespace tos::meta