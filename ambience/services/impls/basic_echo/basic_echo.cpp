#include <echo_generated.hpp>
#include <log_generated.hpp>

namespace {
struct impl : tos::ae::services::echo::sync_server {
    impl(tos::services::logger::sync_server* logger)
        : m_logger{logger} {
    }

    int32_t echo_num(const int32_t& num) override {
        return num;
    }

    std::string_view echo_str(std::string_view data,
                              lidl::message_builder& response_builder) override {
        return data;
    }

    tos::services::logger::sync_server* m_logger;
};
} // namespace

tos::ae::services::echo::sync_server*
init_basic_echo(tos::services::logger::sync_server* logger) {
    return new impl(logger);
}