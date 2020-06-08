#pragma once

#include "sink.hpp"

namespace tos::debug {
template<class SinkT, class ClockT>
class clock_sink_adapter : public detail::any_sink {
public:
    explicit clock_sink_adapter(SinkT sink, ClockT clock)
        : m_sink(std::move(sink))
        , m_clock{std::move(clock)} {
    }
    bool begin(log_level level) override {
        if (!m_sink->begin(level)) {
            return false;
        }
        auto now = std::chrono::duration_cast<std::chrono::seconds>(m_clock.now().time_since_epoch());
        auto str = "[" + std::to_string(now.count()) + "]";
        add(str);
        return true;
    }
    void add(int64_t i) override {
        m_sink->add(i);
    }
    void add(std::string_view str) override {
        m_sink->add(str);
    }
    void add(span<const uint8_t> buf) override {
        m_sink->add(buf);
    }
    void add(bool b) override {
        m_sink->add(b);
    }
    void add(void* ptr) override {
        m_sink->add(ptr);
    }
    void add(log_level level) override {
        m_sink->add(level);
    }
    void add(double d) override {
        m_sink->add(d);
    }
    void end() override {
        m_sink->end();
    }

    SinkT m_sink;
    ClockT m_clock;
};
} // namespace tos::debug