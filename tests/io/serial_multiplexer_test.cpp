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

struct mock_uart_writeonly {
    std::vector<uint8_t> data;

    int write(tos::span<const uint8_t> span) {
        this->data.insert(this->data.end(), span.begin(), span.end());
        return span.size();
    }
};

struct mock_uart_readonly {
    std::deque<uint8_t> data;

    tos::span<uint8_t> read(tos::span<uint8_t> span) {
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
    tos::serial_multiplexer<decltype(&uart)> mp_write(&uart);

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

    // size_t idx = 0;
    // for (auto chr : uart.data) {
    //     fprintf(stdout, "0x%02x, ", (unsigned char)chr);
    //     if (idx++ % 32 == 31) {
    //         std::cout << std::endl;
    //     }
    // }
}

TEST_CASE("serial multiplexer can read from a stream correctly") {
    // setup the write end
    mock_uart_writeonly uart;
    tos::serial_multiplexer<decltype(&uart)> mp_write(&uart);

    constexpr const uint8_t message[] = "hello world!";
    auto stream_write = mp_write.create_stream(0);
    stream_write.write(message);

    // test reading from it
    mock_uart_readonly uart_r;
    uart_r.data.insert(uart_r.data.begin(), uart.data.begin(), uart.data.end());

    tos::serial_multiplexer<decltype(&uart_r)> mp_read(&uart_r);
    auto stream_read = mp_read.create_stream(0);

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

    tos::serial_multiplexer<decltype(&uart)> mp_write(&uart);
    tos::serial_multiplexer<decltype(&uart)> mp_read(&uart);

    constexpr const uint8_t message[] = "hello world!";

    auto stream_write = mp_write.create_stream(0);
    stream_write.write(message);

    auto stream_read = mp_read.create_stream(0);
    std::array<uint8_t, sizeof(message)> result;
    stream_read.read(result);

    REQUIRE_EQ(0, equal(result, message));
}

TEST_CASE("serial multiplexer can read and write to two streams") {
    mock_uart uart;

    tos::serial_multiplexer<decltype(&uart)> mp_write(&uart);
    tos::serial_multiplexer<decltype(&uart)> mp_read(&uart);

    constexpr const uint8_t message0[] = "hello world!";
    constexpr const uint8_t message1[] = "hello world!";

    auto stream_write0 = mp_write.create_stream(0);
    stream_write0.write(message0);
    auto stream_write1 = mp_write.create_stream(1);
    stream_write1.write(message1);

    auto stream_read0 = mp_read.create_stream(0);
    auto stream_read1 = mp_read.create_stream(1);

    std::array<uint8_t, sizeof(message0)> result0;
    stream_read0.read(result0);
    REQUIRE_EQ(0, equal(result0, message0));

    std::array<uint8_t, sizeof(message1)> result1;
    stream_read1.read(result1);
    REQUIRE_EQ(0, equal(result1, message1));
}
} // namespace
