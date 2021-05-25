#pragma once

#include <chrono>
#include <cstdint>
#include <string_view>
#include <tos/debug/detail/log_level.hpp>
#include <tos/task.hpp>

namespace tos::debug::detail {
struct any_async_sink {
    virtual tos::Task<bool> begin(log_level) = 0;
    virtual tos::Task<void> add(int64_t i) = 0;
    virtual tos::Task<void> add(std::string_view str) = 0;
    virtual tos::Task<void> add(tos::span<const uint8_t> buf) = 0;
    virtual tos::Task<void> add(bool b) = 0;
    virtual tos::Task<void> add(const void* ptr) = 0;
    virtual tos::Task<void> add(log_level) = 0;
    virtual tos::Task<void> add(double d) = 0;
    virtual tos::Task<void> end() = 0;
    virtual ~any_async_sink() = default;

    auto add(int i) {
        return add(static_cast<int64_t>(i));
    }

    template<class T = int16_t, typename = std::enable_if_t<!std::is_same<T, int>{}>>
    auto add(int16_t i) {
        return add(static_cast<int64_t>(i));
    }

    template<class T = int32_t, typename = std::enable_if_t<!std::is_same<T, int>{}>>
    auto add(int32_t i) {
        return add(static_cast<int64_t>(i));
    }

    auto add(uint16_t i) {
        return add(static_cast<int64_t>(i));
    }

    auto add(uint32_t i) {
        return add(static_cast<int64_t>(i));
    }

    auto add(uint64_t i) {
        return add(static_cast<int64_t>(i));
    }

    template<class T = size_t,
        typename = std::enable_if_t<!std::is_same<T, uint64_t>{} &&
    !std::is_same<T, uint32_t>{}>>
    auto add(size_t i) {
        return add(static_cast<int64_t>(i));
    }

    auto add(const char* str) {
        return add(static_cast<std::string_view>(str));
    }

    auto add(float f) {
        return add(static_cast<double>(f));
    }

    auto add(void* f) {
        return add(static_cast<const void*>(f));
    }

    tos::Task<void> add(std::chrono::microseconds us) {
        co_await add(static_cast<int64_t>(us.count()));
        co_return co_await add("us");
    }
};

template<class Sink, class... Ts>
tos::Task<void> log_to_async_sink(Sink& sink, log_level level, const Ts&... ts) {
    if (!co_await sink->begin(level)){
        co_return;
    }
    ((co_await sink->add(ts)), ...);
    co_await sink->end();
}
}