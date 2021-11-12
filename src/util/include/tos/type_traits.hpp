#pragma once

#include <type_traits>

namespace tos {
template<template<class...> class Template, class Instance>
struct is_instantiation : std::false_type {};

template<template<class...> class Template, class... Ts>
struct is_instantiation<Template, Template<Ts...>> : std::true_type {};

template <auto X, auto Y>
struct is_same : std::false_type {};

template <auto X>
struct is_same<X, X> : std::true_type {};
} // namespace tos