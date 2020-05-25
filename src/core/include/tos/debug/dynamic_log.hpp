#pragma once

#include <tos/components/component.hpp>
#include <tos/debug/log.hpp>

namespace tos::debug {
struct logger_component : id_component<42> {
    logger_component()
        : logger(&detail::null_log_instance()) {
    }
    detail::any_logger* logger;
};

/**
 * Sets the global log to the given logger instance.
 *
 * Passing `nullptr` as the log will cause the global logging to be disabled.
 *
 * The default log does not participate in the lifetime of the given logger, so
 * the given logger must live unless the default log is set to something else.
 *
 * @param log logger to forward the global logs to
 */
void set_default_log(detail::any_logger* log);
} // namespace tos::debug