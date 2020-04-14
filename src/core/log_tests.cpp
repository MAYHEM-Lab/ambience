#include <deque>
#include <doctest.h>
#include <tos/debug/log.hpp>
#include <tos/debug/sinks/null_sink.hpp>
#include <tos/debug/sinks/serial_sink.hpp>
#include <tos/self_pointing.hpp>
#include <tos/span.hpp>

namespace tos::debug {
namespace {
struct mock_uart : self_pointing<mock_uart> {
    std::deque<char> data;

    int write(tos::span<const uint8_t> span) {
        this->data.insert(this->data.end(), span.begin(), span.end());
        return span.size();
    }

    tos::span<uint8_t> read(tos::span<uint8_t> span) {
        for (auto& chr : span) {
            chr = data.front();
            data.pop_front();
        }
        return span;
    }
};

TEST_CASE("instance logging works") {
    mock_uart serial;
    detail::logger_base log{serial_sink{&serial}};
    log.info("foo");
    REQUIRE_LT(0, serial.data.size());
}

TEST_CASE("global logging works") {
    tos::debug::info("hello", 42);
}
} // namespace
} // namespace tos::debug