#pragma once

#include <chrono>
#include <cstdint>
#include <string_view>
#include <tos/debug/detail/log_level.hpp>
#include <tos/memory.hpp>
#include <tos/error.hpp>

namespace tos::debug::detail {
struct any_sink {
    virtual bool begin(log_level) = 0;
    virtual void add(int64_t i) = 0;
    virtual void add(std::string_view str) = 0;
    virtual void add(span<const uint8_t> buf) = 0;
    virtual void add(bool b) = 0;
    virtual void add(const void* ptr) = 0;
    virtual void add(log_level) = 0;
    virtual void add(double d) = 0;
    virtual void end() = 0;
    virtual ~any_sink() = default;

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

    template<class T = size_t,
             typename = std::enable_if_t<!std::is_same<T, uint64_t>{} &&
                                         !std::is_same<T, uint32_t>{}>>
    void add(size_t i) {
        add(static_cast<int64_t>(i));
    }

    void add(std::chrono::microseconds us) {
        add(static_cast<int64_t>(us.count()));
        add("us");
    }

    void add(const char* str) {
        add(static_cast<std::string_view>(str));
    }

    void add(float f) {
        add(static_cast<double>(f));
    }

    void add(void* f) {
        add(static_cast<const void*>(f));
    }

    void add(physical_address addr) {
        add(reinterpret_cast<const void*>(addr.address()));
    }

    void add(virtual_address addr) {
        add(reinterpret_cast<const void*>(addr.address()));
    }

    void add(const virtual_range& range) {
        add("virtual_range[");
        add(range.base);
        add(", ");
        add(range.end());
        add(")");
    }

    void add(const any_error& err) {
        add(err.name());
        add(":");
        add(err.message());
    }

    template <Error T>
    void add(const T& err) {
        add(err.name());
        add(":");
        add(err.message());
    }
};

template<class Sink, class... Ts>
void log_to_sink(Sink& sink, log_level level, const Ts&... ts) {
    if (!sink->begin(level))
        return;
    (sink->add(ts), ...);
    sink->end();
}
} // namespace tos::debug::detail