//
// Created by fatih on 4/5/20.
//

#include <tos/debug/sinks/null_sink.hpp>
#include <tos/debug/detail/logger_base.hpp>
#include <tos/compiler.hpp>

namespace tos::debug {
namespace detail {
detail::any_logger& null_log_instance() {
    static forget<null_sink> null_sink;
    static detail::any_logger null(&null_sink.get(), log_level::none);
    return null;
}
}

WEAK
detail::any_logger& default_log() {
    return detail::null_log_instance();
}
}