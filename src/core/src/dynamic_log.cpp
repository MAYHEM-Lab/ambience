#include <tos/debug/dynamic_log.hpp>

namespace tos::debug {
namespace {
detail::any_logger* default_log_ptr = nullptr;
}

detail::any_logger& default_log() {
    if (!default_log_ptr) {
        return detail::null_log_instance();
    }

    return *default_log_ptr;
}

void set_default_log(detail::any_logger* log) {
    default_log_ptr = log;
}
}