//
// Created by fatih on 1/5/20.
//

#pragma once

#include <cstdint>
#include "detail/logger_base.hpp"
#include <tos/compiler.hpp>

namespace tos::debug {
detail::any_logger& default_log();

template <class... Ts>
ALWAYS_INLINE
bool log(const Ts&... args) {
#if !defined(TOS_NO_LOG)
    return default_log().log(args...);
#endif
}

template <class... Ts>
ALWAYS_INLINE
bool info(const Ts&... args) {
#if !defined(TOS_NO_LOG)
    return default_log().info(args...);
#endif
}

template <class... Ts>
ALWAYS_INLINE
bool trace(const Ts&... args) {
#if !defined(TOS_NO_LOG)
    return default_log().trace(args...);
#endif
}

template <class... Ts>
ALWAYS_INLINE
bool warn(const Ts&... args) {
#if !defined(TOS_NO_LOG)
    return default_log().warn(args...);
#endif
}

template <class... Ts>
ALWAYS_INLINE
bool error(const Ts&... args) {
#if !defined(TOS_NO_LOG)
    return default_log().error(args...);
#endif
}

template <class... Ts>
ALWAYS_INLINE
bool fatal(const Ts&... args) {
#if !defined(TOS_NO_LOG)
    return default_log().fatal(args...);
#endif
}
} // namespace tos::debug

#define S(x) #x
#define S_(x) S(x)
#define S__LINE__ S_(__LINE__)

#define LOG(...) (::tos::debug::default_log().would_log(::tos::debug::log_level::log) && ::tos::debug::log("[" __FILE__ ":" S__LINE__ "]", __VA_ARGS__))
#define LOG_TRACE(...) (::tos::debug::default_log().would_log(::tos::debug::log_level::trace) && ::tos::debug::trace("[" __FILE__ ":" S__LINE__ "]", __VA_ARGS__))
#define LOG_WARN(...) (::tos::debug::default_log().would_log(::tos::debug::log_level::warning) && ::tos::debug::warn("[" __FILE__ ":" S__LINE__ "]", __VA_ARGS__))
#define LOG_ERROR(...) (::tos::debug::default_log().would_log(::tos::debug::log_level::error) && ::tos::debug::error("[" __FILE__ ":" S__LINE__ "]", __VA_ARGS__))
