#include <iostream>
#include <sstream>
#include <functional>
#include <deque>
#include <tos/span.hpp>
#include <tos/io/serial_multiplexer.hpp>

#include "doctest.h"

namespace {

struct mock_uart {
    std::deque<char> data;

    int write(tos::span<const char> span) {
        this->data.insert(this->data.end(), span.begin(), span.end());
        return span.size();
    }

    tos::span<char> read(tos::span<char> span) {
        for (auto& chr : span) {
            chr = data.front();
            data.pop_front();
        }
        return span;
    }
};

struct mock_uart_writeonly {
    std::vector<char> data;

    int write(tos::span<const char> span) {
        this->data.insert(this->data.end(), span.begin(), span.end());
        return span.size();
    }
};

struct mock_uart_readonly {
    std::deque<char> data;

    tos::span<char> read(tos::span<char> span) {
        for (auto& chr : span) {
            chr = data.front();
            data.pop_front();
        }
        return span;
    }
};

template<typename NumType>
std::array<char, sizeof(NumType)> num_to_chars(NumType num) {
    std::array<char, sizeof(NumType)> arr;
    memcpy(arr.data(), &num, sizeof(num));
    return arr;
}


TEST_CASE("serial multiplexer can write to a stream correctly") {
    mock_uart_writeonly uart;
    tos::serial_multiplexer<decltype(&uart), 512> mp_write(&uart);

    constexpr const char message[] = "hello world!";

    auto stream_write = mp_write.create_stream(0);
    stream_write.write(message);

    REQUIRE_EQ(8 + 2 + 2 + 4 + sizeof(message), uart.data.size());

    REQUIRE(std::equal(uart.data.begin(), uart.data.begin() + 8, mp_write.magic_numbers.begin()));
    REQUIRE(std::equal(uart.data.begin() + 8, uart.data.begin() + 10, num_to_chars<uint16_t>(0).begin()));
    REQUIRE(std::equal(uart.data.begin() + 10, uart.data.begin() + 12, num_to_chars<uint16_t>(sizeof(message)).begin()));
    REQUIRE(std::equal(uart.data.begin() + 12, uart.data.begin() + 12 + sizeof(message), std::begin(message)));
    REQUIRE(
        std::equal(uart.data.begin() + 12 + sizeof(message), uart.data.begin() + 12 + sizeof(message) + 4,
        num_to_chars<uint32_t>(tos::crc32(message)).begin())
    );

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
    tos::serial_multiplexer<decltype(&uart), 512> mp_write(&uart);

    constexpr const char message[] = "hello world!";
    auto stream_write = mp_write.create_stream(0);
    stream_write.write(message);

    // test reading from it 
    mock_uart_readonly uart_r;
    uart_r.data.insert(uart_r.data.begin(), uart.data.begin(), uart.data.end());

    tos::serial_multiplexer<decltype(&uart_r), 512> mp_read(&uart_r);
    auto stream_read = mp_read.create_stream(0);

    std::array<char, sizeof(message)> message_back;
    REQUIRE_EQ(sizeof(message), stream_read.read(message_back).size());
}

TEST_CASE("serial multiplexer can read and write to a single stream") {
    mock_uart uart;

    tos::serial_multiplexer<decltype(&uart), 512> mp_write(&uart);
    tos::serial_multiplexer<decltype(&uart), 512> mp_read(&uart);

    auto stream_write = mp_write.create_stream(0);
    stream_write.write("hello world!");

    auto stream_read = mp_read.create_stream(0);
    std::array<char, sizeof("hello world!")> result;
    stream_read.read(result);

    REQUIRE_EQ(0, strcmp(&result[0], "hello world!"));
}

TEST_CASE("serial multiplexer can read and write to two streams") {
    mock_uart uart;

    tos::serial_multiplexer<decltype(&uart), 512> mp_write(&uart);
    tos::serial_multiplexer<decltype(&uart), 512> mp_read(&uart);

    auto stream_write0 = mp_write.create_stream(0);
    stream_write0.write("hello world0!");
    auto stream_write1 = mp_write.create_stream(1);
    stream_write1.write("hello world1!");

    auto stream_read0 = mp_read.create_stream(0);
    auto stream_read1 = mp_read.create_stream(1);

    std::array<char, sizeof("hello world0!")> result0;
    stream_read0.read(result0);
    REQUIRE_EQ(0, strcmp(&result0[0], "hello world0!"));

    std::array<char, sizeof("hello world1!")> result1;
    stream_read1.read(result1);
    REQUIRE_EQ(0, strcmp(&result1[0], "hello world1!"));
}


}
