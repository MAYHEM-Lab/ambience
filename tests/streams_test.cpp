//
// Created by fatih on 11/7/19.
//

#include <deque>
#include <doctest.h>
#include <tos/self_pointing.hpp>
#include <tos/streams.hpp>

namespace tos {
namespace {
struct mock_uart : self_pointing<mock_uart> {
    std::deque<char> data;

    int write(tos::span<const char> span) {
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

TEST_CASE("read_until works with correct input of size 1") {
    mock_uart mock;
    mock.write("f\ny");

    std::array<char, 8> buf;
    auto sep = '\n';
    auto read = tos::read_until<char>(mock, tos::monospan(sep), buf);
    REQUIRE_EQ(2, read.size());
    REQUIRE_EQ('f', read[0]);
    REQUIRE_EQ('\n', read[1]);
}

TEST_CASE("read_until works with CR/LF") {
    mock_uart mock;
    mock.write("f\r\ny");

    std::array<char, 8> buf;
    char sep[] = {'\r', '\n'};
    auto read = tos::read_until<char>(mock, sep, buf);
    REQUIRE_EQ(3, read.size());
    REQUIRE_EQ('f', read[0]);
    REQUIRE_EQ('\r', read[1]);
    REQUIRE_EQ('\n', read[2]);
}

TEST_CASE("read_until returns correctly on exact fit") {
    mock_uart mock;
    const char msg[] = "hellow\r\n";
    mock.write(msg);
    std::array<char, 8> buf;
    char sep[] = {'\r', '\n'};
    auto read = tos::read_until<char>(mock, sep, buf);
    REQUIRE_EQ(buf.size(), read.size());
    REQUIRE_EQ(tos::span(msg).slice(0, read.size()), read);
}

TEST_CASE("read_until returns if buffer is full") {
    mock_uart mock;
    const char msg[] = "hello world foo bar\r\n";
    mock.write(msg);
    std::array<char, 8> buf;
    char sep[] = {'\r', '\n'};
    auto read = tos::read_until<char>(mock, sep, buf);
    REQUIRE_EQ(buf.size(), read.size());
    REQUIRE_EQ(tos::span(msg).slice(0, read.size()), read);
}
} // namespace
} // namespace tos