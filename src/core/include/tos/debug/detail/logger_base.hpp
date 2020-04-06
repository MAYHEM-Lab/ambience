//
// Created by fatih on 1/5/20.
//

#pragma once

#include "log_level.hpp"
#include "log_message.hpp"

#include <tos/compiler.hpp>
#include <tos/debug/sinks/sink.hpp>
#include <utility>

namespace tos::debug::detail {
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
    ALWAYS_INLINE bool info(const Ts&... args) {
        if (!would_log(log_level::info)) {
            return false;
        }
        log_to_sink(m_sink, log_level::info, args...);
        return true;
    }

    template<class... Ts>
    ALWAYS_INLINE bool warn(const Ts&... args) {
        if (!would_log(log_level::warning)) {
            return false;
        }
        log_to_sink(m_sink, log_level::warning, args...);
        return true;
    }

    template<class... Ts>
    ALWAYS_INLINE bool trace(const Ts&... args) {
        if (!would_log(log_level::trace)) {
            return false;
        }
        log_to_sink(m_sink, log_level::trace, args...);
        return true;
    }

    template<class... Ts>
    ALWAYS_INLINE bool log(const Ts&... args) {
        if (!would_log(log_level::log)) {
            return false;
        }
        log_to_sink(m_sink, log_level::log, args...);
        return true;
    }

    template<class... Ts>
    ALWAYS_INLINE bool fatal(const Ts&... args) {
        if (!would_log(log_level::fatal)) {
            return false;
        }
        log_to_sink(m_sink, log_level::fatal, args...);
        return true;
    }

    [[nodiscard]] ALWAYS_INLINE bool would_log(log_level level) const {
        return m_level >= level;
    }

    ALWAYS_INLINE
    void set_log_level(log_level level) {
        m_level = level;
    }

private:
    SinkT m_sink;
    log_level m_level;
};

class any_logger : public logger_base<any_sink*, dynamic_enable> {
public:
    using logger_base::logger_base;
};
} // namespace tos::debug::detail