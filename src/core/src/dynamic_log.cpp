#include <tos/context.hpp>
#include <tos/debug/dynamic_log.hpp>

namespace tos::debug {
detail::any_logger& default_log() {
    auto component = current_context().get_component<logger_component>();
    return *component->logger;
}

void set_default_log(detail::any_logger* log) {
    if (auto component = current_context().get_component<logger_component>(); component) {
        component->logger = log;
    }
}
} // namespace tos::debug