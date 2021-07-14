#include <alarm_generated.hpp>
#include <calc_generated.hpp>
#include <file_system_generated.hpp>
#include <log_generated.hpp>
#include <tos/debug/sinks/lidl_sink.hpp>
#include <tos/detail/poll.hpp>
#include <tos/mem_stream.hpp>
#include <tos/print.hpp>
#include <tos/task.hpp>

namespace {
struct impl : tos::ae::services::calculator::async_server {
    impl(tos::services::logger::async_server* logger,
         tos::ae::services::alarm::async_server* alarm,
         tos::ae::services::filesystem::async_server* fs)
        : m_logger(logger)
        , m_alarm(alarm)
        , m_fs(fs)
        , m_sink(*m_logger) {
    }

    tos::Task<int32_t> add(const int32_t& x, const int32_t& y) override {
//        tos::coro::make_detached([this]() -> tos::Task<void> {
//            auto ptr = &m_sink;
//            co_await log_to_async_sink(
//                ptr, tos::debug::log_level::info, std::string_view("in add"));
//        }());
        co_return x + y;
    }

    tos::services::logger::async_server* m_logger;
    tos::ae::services::alarm::async_server* m_alarm;
    tos::ae::services::filesystem::async_server* m_fs;
    tos::debug::async_lidl_sink m_sink;
};
} // namespace

tos::Task<tos::ae::services::calculator::async_server*>
init_basic_calc(tos::services::logger::async_server* logger,
                tos::ae::services::alarm::async_server* alarm,
                tos::ae::services::filesystem::async_server* fs) {
    co_return new impl(logger, alarm, fs);
}