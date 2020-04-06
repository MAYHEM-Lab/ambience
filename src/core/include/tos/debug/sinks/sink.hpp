#pragma once

#include <string_view>
#include <cstdint>

namespace tos::debug::detail {
struct any_sink {
    virtual bool begin() = 0;
    virtual void add(int64_t i) = 0;
    virtual void add(std::string_view str) = 0;
    virtual void end() = 0;

    virtual ~any_sink() = default;
};

template<class Sink, class... Ts>
void log(Sink& sink, const Ts&... ts) {
    if (!sink->begin()) return;
    (sink->add(ts), ...);
    sink->end();
}
}