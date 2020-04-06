//
// Created by fatih on 1/5/20.
//

#pragma once

#include <cstdint>
#include "detail/logger_base.hpp"

namespace tos::debug {
detail::any_logger& default_log();

template <class... Ts>
void info(const Ts&... args) {
#if !defined(TOS_NO_LOG)
    default_log().info(args...);
#endif
}
} // namespace tos::debug