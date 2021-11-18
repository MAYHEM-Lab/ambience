#pragma once

#include <tos/meta/types.hpp>

namespace tos::meta {
template<template<class...> class Ins, class...>
struct instantiate;

template<template<class...> class Ins, class... Ts>
struct instantiate<Ins, list<Ts...>> {
    using type = Ins<Ts...>;
};

template<template<class...> class Ins, class... Ts>
using instantiate_t = typename instantiate<Ins, Ts...>::type;
} // namespace tos::meta