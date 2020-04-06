#pragma once

#include <cstdint>
#include <tos/print.hpp>

namespace tos::debug {
enum class log_level : uint8_t
{
    fatal,
    error,
    warning,
    log,
    debug,
    info,
    trace,
    all = trace
};

template <class SerialT>
void print(SerialT& serial, log_level level) {
    using tos::print;
    print(serial, [level]{
        switch(level) {
        case log_level::fatal: return "fatal";
        case log_level::error: return "error";
        case log_level::warning: return "warning";
        case log_level::log: return "log";
        case log_level::debug: return "debug";
        case log_level::info: return "info";
        case log_level::trace: return "trace";
        default: return "unknown";
        }
    }());
}
}