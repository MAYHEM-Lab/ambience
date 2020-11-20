#pragma once

#include <log_generated.hpp>
#include <tos/debug/sinks/sink.hpp>
#include <tos/mutex.hpp>

namespace tos::debug {
class log_server : public services::logger {
public:
    explicit log_server(tos::debug::detail::any_sink& sink)
        : m_sink{&sink} {
    }

    bool start(services::log_level level) override;
    bool finish() override;
    bool log_int(int64_t val) override;
    bool log_float(double val) override;
    bool log_bool(bool val) override;
    bool log_string(std::string_view val) override;
    bool log_pointer(uint64_t val) override;
    bool log_log_level(services::log_level val) override;

private:
    tos::debug::detail::any_sink* m_sink;
};

class lidl_sink : public detail::any_sink {
public:
    explicit lidl_sink(services::logger& log)
        : m_logger{&log} {
    }

    bool begin(log_level level) override;
    void add(int64_t i) override;
    void add(std::string_view str) override;
    void add(span<const uint8_t> buf) override;
    void add(bool b) override;
    void add(void* ptr) override;
    void add(log_level level) override;
    void add(double d) override;
    void end() override;

private:
    mutex m_prot;
    services::logger* m_logger;
};
} // namespace tos::debug