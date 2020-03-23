#include "doctest.h"

#include <deque>
#include <functional>
#include <iostream>
#include <sstream>
#include <tos/io/serial_multiplexer.hpp>
#include <tos/span.hpp>

namespace {

struct mock_uart {
    std::deque<char> data;
    tos::semaphore len{0};
    bool m_eof = false;

    void eof() {
        m_eof = true;
        len.up();
    }

    int write(tos::span<const uint8_t> span) {
        this->data.insert(this->data.end(), span.begin(), span.end());
        up_many(len, span.size());
        return span.size();
    }

    tos::span<uint8_t> read(tos::span<uint8_t> span) {
        int length = 0;
        for (auto& chr : span) {
            len.down();
            if (m_eof && get_count(len) == 0) {
                len.up();
                return span.slice(0, length);
            }
            chr = data.front();
            data.pop_front();
            ++length;
        }
        return span;
    }
};

struct mock_uart_writeonly {
    std::vector<uint8_t> data;

    int write(tos::span<const uint8_t> span) {
        this->data.insert(this->data.end(), span.begin(), span.end());
        return span.size();
    }

    tos::span<uint8_t> read(tos::span<uint8_t>) {
        return tos::span<uint8_t>{nullptr};
    }
};

struct mock_uart_readonly {
    std::deque<uint8_t> data;

    int write(tos::span<const uint8_t>) {
        return 0;
    }

    tos::span<uint8_t> read(tos::span<uint8_t> span) {
        if (data.empty()) {
            return tos::span<uint8_t>{nullptr};
        }
        for (auto& chr : span) {
            chr = data.front();
            data.pop_front();
        }
        return span;
    }
};

template<typename NumType>
std::array<uint8_t, sizeof(NumType)> num_to_chars(NumType num) {
    std::array<uint8_t, sizeof(NumType)> arr;
    memcpy(arr.data(), &num, sizeof(num));
    return arr;
}


TEST_CASE("serial multiplexer can write to a stream correctly") {
    mock_uart_writeonly uart;
    tos::serial_multiplexer mp_write(&uart, true);

    constexpr const uint8_t message[] = "hello world!";

    auto stream_write = mp_write.create_stream(0);
    stream_write.write(message);

    REQUIRE_EQ(8 + 2 + 2 + 4 + sizeof(message), uart.data.size());

    REQUIRE(std::equal(
        uart.data.begin(), uart.data.begin() + 8, mp_write.magic_numbers.begin()));
    REQUIRE(std::equal(uart.data.begin() + 8,
                       uart.data.begin() + 10,
                       num_to_chars<uint16_t>(0).begin()));
    REQUIRE(std::equal(uart.data.begin() + 10,
                       uart.data.begin() + 12,
                       num_to_chars<uint16_t>(sizeof(message)).begin()));
    REQUIRE(std::equal(uart.data.begin() + 12,
                       uart.data.begin() + 12 + sizeof(message),
                       std::begin(message)));
    REQUIRE(std::equal(uart.data.begin() + 12 + sizeof(message),
                       uart.data.begin() + 12 + sizeof(message) + 4,
                       num_to_chars<uint32_t>(
                           tos::crc32(tos::raw_cast<const uint8_t>(tos::span(message))))
                           .begin()));
}

TEST_CASE("serial multiplexer can read from a stream correctly") {
    // setup the write end
    mock_uart_writeonly uart;
    tos::serial_multiplexer mp_write(&uart, true);

    constexpr const uint8_t message[] = "hello world!";
    auto stream_write = mp_write.create_stream(0);
    stream_write.write(message);

    // test reading from it
    mock_uart_readonly uart_r;
    uart_r.data.insert(uart_r.data.begin(), uart.data.begin(), uart.data.end());

    tos::serial_multiplexer mp_read(&uart_r, {0});
    auto stream_read = *mp_read.get_stream(0);

    std::array<uint8_t, sizeof(message)> message_back;
    REQUIRE_EQ(sizeof(message), stream_read.read(message_back).size());
}

template<class LeftT, class RightT>
bool equal(const LeftT& left_range, const RightT& right_range) {
    return std::equal(
        std::begin(left_range), std::end(left_range), std::begin(right_range));
}

TEST_CASE("serial multiplexer can read and write to a single stream") {
    mock_uart uart;

    tos::serial_multiplexer mp_write(&uart, {0}, true);
    tos::serial_multiplexer mp_read(&uart, {0});

    constexpr const uint8_t message[] = "hello world!";

    auto stream_write = *mp_write.get_stream(0);
    stream_write.write(message);

    auto stream_read = *mp_read.get_stream(0);
    std::array<uint8_t, sizeof(message)> result;
    stream_read.read(result);

    REQUIRE(equal(result, message));
}

TEST_CASE("serial multiplexer can read and write to two streams") {
    mock_uart uart;

    tos::serial_multiplexer mp_write(&uart, {0, 1}, true);
    tos::serial_multiplexer mp_read(&uart, {0, 1});

    constexpr const uint8_t message0[] = "hello world!";
    constexpr const uint8_t message1[] = "hello world!";

    auto stream_write0 = *mp_write.get_stream(0);
    stream_write0.write(message0);
    auto stream_write1 = *mp_write.get_stream(1);
    stream_write1.write(message1);

    auto stream_read0 = *mp_read.get_stream(0);
    auto stream_read1 = *mp_read.get_stream(1);

    std::array<uint8_t, sizeof(message0)> result0;
    stream_read0.read(result0);
    REQUIRE(equal(result0, message0));

    std::array<uint8_t, sizeof(message1)> result1;
    stream_read1.read(result1);
    REQUIRE(equal(result1, message1));
}
} // namespace
