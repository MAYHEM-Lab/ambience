#pragma once

namespace tos::meta {
template<class T>
struct identity {
    using type = T;
};

template<class T>
using id = identity<T>;

template<class...>
struct list {};

template<auto...>
struct values {};

template<template<class...> class, class>
struct apply;

template<template<class...> class To, class... Ts>
struct apply<To, list<Ts...>> {
    using type = To<Ts...>;
};

template<template<class...> class To, class List>
using apply_t = typename apply<To, List>::type;
} // namespace tos::meta