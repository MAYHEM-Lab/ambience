//
// Created by fatih on 1/5/20.
//

#pragma once

#include "detail/logger_base.hpp"
#include <cstdint>
#include <tos/compiler.hpp>
#include <utility>
#include <fmt/compile.h>

namespace tos::debug {
detail::any_logger& default_log();

template<class... Ts>
ALWAYS_INLINE bool log(const Ts&... args) {
#if !defined(TOS_NO_LOG)
    return default_log().log(args...);
#else
    return false;
#endif
}

template<class... Ts>
ALWAYS_INLINE bool info(const Ts&... args) {
#if !defined(TOS_NO_LOG)
    return default_log().info(args...);
#else
    return false;
#endif
}

template<class... Ts>
ALWAYS_INLINE bool trace(const Ts&... args) {
#if !defined(TOS_NO_LOG)
    return default_log().trace(args...);
#else
    return false;
#endif
}

template<class... Ts>
ALWAYS_INLINE bool warn(const Ts&... args) {
#if !defined(TOS_NO_LOG)
    return default_log().warn(args...);
#else
    return false;
#endif
}

template<class... Ts>
ALWAYS_INLINE bool error(const Ts&... args) {
#if !defined(TOS_NO_LOG)
    return default_log().error(args...);
#else
    return false;
#endif
}

template<class... Ts>
ALWAYS_INLINE bool fatal(const Ts&... args) {
#if !defined(TOS_NO_LOG)
    return default_log().fatal(args...);
#else
    return false;
#endif
}

template<template<class...> class TupleT, class... Ts>
bool log_tuple(const TupleT<Ts...>& tuple) {
    return [&]<size_t... Is>(std::index_sequence<Is...>) {
        return log(get<Is>(tuple)...);
    }
    (std::make_index_sequence<sizeof...(Ts)>());
}
} // namespace tos::debug

#define __S(x)      #x
#define __S_(x)     __S(x)
#define __S__LINE__ __S_(__LINE__)

#if !defined(TOS_NO_LOG)
#define LOG(...)                                                            \
    (::tos::debug::default_log().would_log(::tos::debug::log_level::log) && \
     ::tos::debug::log("[" __FILE__ ":" __S__LINE__ "]", __VA_ARGS__))
#define LOG_TRACE(...)                                                        \
    (::tos::debug::default_log().would_log(::tos::debug::log_level::trace) && \
     ::tos::debug::trace("[" __FILE__ ":" __S__LINE__ "]", __VA_ARGS__))
#define LOG_INFO(...)                                                        \
    (::tos::debug::default_log().would_log(::tos::debug::log_level::info) && \
     ::tos::debug::info("[" __FILE__ ":" __S__LINE__ "]", __VA_ARGS__))
#define LOG_WARN(...)                                                           \
    (::tos::debug::default_log().would_log(::tos::debug::log_level::warning) && \
     ::tos::debug::warn("[" __FILE__ ":" __S__LINE__ "]", __VA_ARGS__))
#define LOG_ERROR(...)                                                        \
    (::tos::debug::default_log().would_log(::tos::debug::log_level::error) && \
     ::tos::debug::error("[" __FILE__ ":" __S__LINE__ "]", __VA_ARGS__))
#define LOG_FORMAT(format_str, ...) LOG(fmt::format(FMT_COMPILE(format_str), __VA_ARGS__))
#else
#define LOG(...)       false
#define LOG_TRACE(...) false
#define LOG_INFO(...)  false
#define LOG_WARN(...)  false
#define LOG_ERROR(...) false
#endif