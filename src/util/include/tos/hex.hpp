#pragma once

#include <tos/compiler.hpp>
#include <tos/span.hpp>

namespace tos {
namespace detail {
constexpr uint8_t nibble_to_int(char nibble) {
    switch (nibble) {
    case '0':
        return 0;
    case '1':
        return 1;
    case '2':
        return 2;
    case '3':
        return 3;
    case '4':
        return 4;
    case '5':
        return 5;
    case '6':
        return 6;
    case '7':
        return 7;
    case '8':
        return 8;
    case '9':
        return 9;
    case 'a':
    case 'A':
        return 10;
    case 'b':
    case 'B':
        return 11;
    case 'c':
    case 'C':
        return 12;
    case 'd':
    case 'D':
        return 13;
    case 'e':
    case 'E':
        return 14;
    case 'f':
    case 'F':
        return 15;
    }
    TOS_UNREACHABLE();
}

constexpr uint8_t byte_from_hex(tos::span<const char> nibbles) {
    auto left_nibble = nibble_to_int(nibbles[0]);
    auto right_nibble = nibble_to_int(nibbles[1]);
    return (left_nibble << 4) | right_nibble;
}
} // namespace detail


template<size_t N>
constexpr std::array<uint8_t, N / 2> hex_to_bytes(const char (&str)[N]) {
    static_assert((N & 1) == 1, "Hex string must have an even number of characters, excluding the null terminator.");
    std::array<uint8_t, N / 2> res;
    for (size_t i = 0; i + 1 < N; i += 2) {
        res[i / 2] = detail::byte_from_hex(span(str).slice(i, 2));
    }
    return res;
}
} // namespace tos
