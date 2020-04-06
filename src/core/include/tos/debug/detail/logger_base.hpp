//
// Created by fatih on 1/5/20.
//

#pragma once

#include "log_message.hpp"

#include <tos/debug/sinks/sink.hpp>
#include <utility>

namespace tos::debug {
enum class log_level : uint8_t
{
    error,
    warning,
    debug,
    log,
    info,
    all = info
};

namespace detail {
struct dynamic_enable {
protected:
    explicit dynamic_enable(bool enabled = true)
        : m_enabled(enabled) {
    }

public:
    [[nodiscard]] bool enabled() const {
        return m_enabled;
    }

    void set_enabled(bool status) {
        m_enabled = status;
    }

private:
    bool m_enabled;
};

template<bool Enabled>
struct static_enable {
    [[nodiscard]] bool enabled() const {
        return Enabled;
    }
};

template<class SinkT, class Enabler = dynamic_enable>
class logger_base : public Enabler {
public:
    explicit logger_base(SinkT sink)
        : m_sink(std::move(sink))
        , m_level{log_level::all} {
    }

    template<class... Ts>
    bool info(const Ts&... args) {
        log(m_sink, args...);
        return true;
    }

    template<class... Ts>
    bool warn(const Ts&... args) {
        log(m_sink, args...);
        return true;
    }

private:
    SinkT m_sink;
    log_level m_level;
};

class any_logger : public logger_base<any_sink*, dynamic_enable> {
public:
    using logger_base::logger_base;
};
} // namespace detail
} // namespace tos::debug