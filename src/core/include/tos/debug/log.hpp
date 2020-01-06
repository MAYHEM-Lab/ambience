//
// Created by fatih on 1/5/20.
//

#pragma once

#include <cstdint>

#if !__has_include("tos/debug/default_logger.hpp")
#define TOS_NODEFLOG
#endif

namespace tos::debug {
enum class log_level : uint8_t
{
    error,
    warning,
    debug,
    log,
    info,
    all = info
};

class null_logger {
public:
    [[nodiscard]] bool enabled() const {
        return false;
    }

    template<class... Ts>
    void log(Ts&&...) {

    }
};

auto default_log(log_level) {
#if defined(TOS_NODEFLOG)
    return null_logger();
#endif
}
} // namespace tos::debug

#define LOG_LEVEL(log_level)                                                             \
    (tos::debug::default_log(log_level).enabled()) && tos::debug::default_log(log_level)

#define LOG LOG_LEVEL(tos::debug::log_level::log)
#define WARN LOG_LEVEL(tos::debug::log_level::warning)
#define ERROR LOG_LEVEL(tos::debug::log_level::error)

#define LOG_IF(cond) ((cond)) && LOG
#define WARN_IF(cond) ((cond)) && WARN
#define ERROR_IF(cond) ((cond)) && ERROR