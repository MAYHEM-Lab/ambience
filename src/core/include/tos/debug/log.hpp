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
bool fatal(const Ts&... args) {
#if !defined(TOS_NO_LOG)
    return default_log().fatal(args...);
#endif
}
} // namespace tos::debug

#define LOG_TRACE(...) (::tos::debug::default_log().would_log(::tos::debug::log_level::trace) && ::tos::debug::trace(__VA_ARGS__))