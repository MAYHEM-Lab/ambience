#pragma once

#include <chrono>
#include <cstdint>
#include <string_view>
#include <tos/debug/detail/log_level.hpp>

namespace tos::debug::detail {
struct any_sink {
    virtual bool begin(log_level) = 0;
    virtual void add(int64_t i) = 0;
    virtual void add(std::string_view str) = 0;
    virtual void add(span<const uint8_t> buf) = 0;
    virtual void add(bool b) = 0;
    virtual void add(void* ptr) = 0;
    virtual void add(log_level) = 0;

    void add(int i) {
        add(static_cast<int64_t>(i));
    }

    template<class T = int16_t, typename = std::enable_if_t<!std::is_same<T, int>{}>>
    void add(int16_t i) {
        add(static_cast<int64_t>(i));
    }

    template<class T = int32_t, typename = std::enable_if_t<!std::is_same<T, int>{}>>
    void add(int32_t i) {
        add(static_cast<int64_t>(i));
    }

    void add(uint16_t i) {
        add(static_cast<int64_t>(i));
    }

    void add(uint32_t i) {
        add(static_cast<int64_t>(i));
    }

    void add(uint64_t i) {
        add(static_cast<int64_t>(i));
    }

    template<class T = uintptr_t,
             typename = std::enable_if_t<!std::is_same<T, uint64_t>{} &&
                                         !std::is_same<T, uint32_t>{}>>
    void add(uintptr_t i) {
        add(reinterpret_cast<void*>(i));
    }

    void add(std::chrono::microseconds us) {
        add(static_cast<int64_t>(us.count()));
        add("us");
    }

    void add(const char* str) {
        add(static_cast<std::string_view>(str));
    }

    virtual void end() = 0;

    virtual ~any_sink() = default;
};

template<class Sink, class... Ts>
void log_to_sink(Sink& sink, log_level level, const Ts&... ts) {
    if (!sink->begin(level))
        return;
    (sink->add(ts), ...);
    sink->end();
}
} // namespace tos::debug::detail