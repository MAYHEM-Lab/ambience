#include <calc_generated.hpp>
#include <log_generated.hpp>
#include <tos/task.hpp>

namespace {
struct impl : tos::ae::services::calculator::async_server {
    impl(tos::services::logger::async_server* logger)
        : m_logger(logger) {
    }
    tos::Task<int32_t> add(const int32_t& x, const int32_t& y) override {
        co_await m_logger->start(tos::services::log_level::info);
        co_await m_logger->log_string("in add");
        co_await m_logger->finish();
        co_return x + y;
    }

    tos::services::logger::async_server* m_logger;
};
} // namespace

tos::Task<tos::ae::services::calculator::async_server*>
init_basic_calc(tos::services::logger::async_server* logger) {
    co_return new impl(logger);
}