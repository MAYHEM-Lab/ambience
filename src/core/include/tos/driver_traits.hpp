//
// Created by fatih on 8/22/18.
//

#pragma once

#include <tos/span.hpp>

namespace tos {
namespace details {
template<class, class, class... Args>
struct read_impl_exists : std::false_type {};
template<class Arg, class... Args>
struct read_impl_exists<
    void,
    std::void_t<decltype(std::declval<Arg>().read(std::declval<Args>()...))>,
    Arg,
    Args...> : std::true_type {};

template<class, class, class... Args>
struct write_impl_exists : std::false_type {};
template<class Arg, class... Args>
struct write_impl_exists<
    void,
    std::void_t<decltype(std::declval<Arg>().write(std::declval<Args>()...))>,
    Arg,
    Args...> : std::true_type {};
} // namespace details

template<class... Args>
using read_impl_exists = typename details::read_impl_exists<void, void, Args...>::type;

template<class... Args>
using write_impl_exists = typename details::write_impl_exists<void, void, Args...>::type;

template<class T>
struct driver_traits {
    using has_read = read_impl_exists<T, tos::span<uint8_t>>;
    using has_write = write_impl_exists<T, tos::span<const uint8_t>>;
};

template<class T>
static constexpr auto has_read_v = bool(typename driver_traits<T>::has_read{});

template<class T>
static constexpr auto has_write_v = bool(typename driver_traits<T>::has_write{});
} // namespace tos
