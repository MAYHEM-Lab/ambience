#pragma once

#include <tos/debug/log.hpp>

namespace tos::debug {
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
}