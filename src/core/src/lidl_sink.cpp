#include <tos/debug/sinks/lidl_sink.hpp>
#include <tos/compiler.hpp>

namespace tos::debug {
namespace {
log_level convert(services::log_level level) {
    switch (level) {
    case services::log_level::none:
        return log_level::none;
    case services::log_level::trace:
        return log_level::trace;
    case services::log_level::debug:
        return log_level::debug;
    case services::log_level::info:
        return log_level::info;
    case services::log_level::log:
        return log_level::log;
    case services::log_level::warning:
        return log_level::warning;
    case services::log_level::error:
        return log_level::error;
    case services::log_level::fatal:
        return log_level::fatal;
    }
    TOS_UNREACHABLE();
}

services::log_level convert_back(log_level level) {
    switch (level) {
    case log_level::none:
        return services::log_level::none;
    case log_level::fatal:
        return services::log_level::fatal;
    case log_level::error:
        return services::log_level::error;
    case log_level::warning:
        return services::log_level::warning;
    case log_level::log:
        return services::log_level::log;
    case log_level::debug:
        return services::log_level::debug;
    case log_level::info:
        return services::log_level::info;
    case log_level::trace:
        return services::log_level::trace;
    }
    TOS_UNREACHABLE();
}
}
bool log_server::start(const services::log_level& level) {
    return m_sink->begin(convert(level));
}

bool log_server::log_pointer(const uint64_t& val) {
    m_sink->add(reinterpret_cast<void*>(val));
    return true;
}

bool log_server::finish() {
    m_sink->end();
    return true;
}

bool log_server::log_int(const int64_t& val) {
    m_sink->add(val);
    return true;
}

bool log_server::log_bool(const bool& val) {
    m_sink->add(val);
    return true;
}

bool log_server::log_string(std::string_view val) {
    m_sink->add(val);
    return true;
}

bool log_server::log_float(const double& val) {
    m_sink->add(val);
    return true;
}
bool log_server::log_log_level(const services::log_level& val) {
    m_sink->add(convert(val));
    return true;
}

bool lidl_sink::begin(log_level level) {
    auto res = m_logger->start(convert_back(level));
    if (res) {
        m_prot.lock();
    }
    return res;
}

void lidl_sink::add(int64_t i) {
    m_logger->log_int(i);
}

void lidl_sink::add(std::string_view str) {
    m_logger->log_string(str);
}

void lidl_sink::add(span<const uint8_t> buf) {
    // TODO
}

void lidl_sink::add(bool b) {
    m_logger->log_bool(b);
}

void lidl_sink::add(void* ptr) {
    m_logger->log_pointer(reinterpret_cast<uint64_t>(ptr));
}

void lidl_sink::add(log_level level) {
    m_logger->log_log_level(convert_back(level));
}

void lidl_sink::add(double d) {
    m_logger->log_float(d);
}

void lidl_sink::end() {
    m_logger->finish();
    m_prot.unlock();
}
} // namespace tos::debug