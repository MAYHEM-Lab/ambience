#pragma once

#include <log_generated.hpp>
#include <tos/debug/sinks/sink.hpp>
#include <tos/debug/sinks/async_sink.hpp>

namespace tos::debug {
class log_server : public services::logger::sync_server {
public:
    explicit log_server(tos::debug::detail::any_sink& sink)
        : m_sink{&sink} {
    }

    bool start(services::log_level level) override;
    bool finish() override;
    bool log_int(const int64_t& val) override;
    bool log_float(const double& val) override;
    bool log_bool(bool val) override;
    bool log_string(std::string_view val) override;
    bool log_pointer(const uint64_t& val) override;
    bool log_log_level(services::log_level val) override;

private:
    tos::debug::detail::any_sink* m_sink;
};

class lidl_sink : public detail::any_sink {
public:
    explicit lidl_sink(services::logger::sync_server& log)
        : m_logger{&log} {
    }

    bool begin(log_level level) override;
    void add(int64_t i) override;
    void add(std::string_view str) override;
    void add(span<const uint8_t> buf) override;
    void add(bool b) override;
    void add(const void* ptr) override;
    void add(log_level level) override;
    void add(double d) override;
    void end() override;

private:
    services::logger::sync_server* m_logger;
};

class async_lidl_sink : public detail::any_async_sink {
public:
    explicit async_lidl_sink(services::logger::async_server& log)
        : m_logger{&log} {
    }

    tos::Task<bool> begin(log_level level) override;
    tos::Task<void> add(int64_t i) override;
    tos::Task<void> add(std::string_view str) override;
    tos::Task<void> add(span<const uint8_t> buf) override;
    tos::Task<void> add(bool b) override;
    tos::Task<void> add(const void* ptr) override;
    tos::Task<void> add(log_level level) override;
    tos::Task<void> add(double d) override;
    tos::Task<void> end() override;

private:
    services::logger::async_server* m_logger;
};
} // namespace tos::debug