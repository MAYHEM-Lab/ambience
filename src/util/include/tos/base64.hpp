#pragma once
#include <cstdint>
#include <tos/span.hpp>
#include <vector>

namespace tos {
namespace detail {
constexpr const char* B64chars =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
constexpr const int B64index[256] = {
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  62, 63, 62, 62, 63, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 0,  0,  0,  0,  0,
    0,  0,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15, 16, 17, 18,
    19, 20, 21, 22, 23, 24, 25, 0,  0,  0,  0,  63, 0,  26, 27, 28, 29, 30, 31, 32, 33,
    34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51};

} // namespace detail

inline std::vector<uint8_t> base64encode(span<const uint8_t> data) {
    std::vector<uint8_t> result((data.size_bytes() + 2) / 3 * 4, '=');
    auto* p = static_cast<const uint8_t*>(data.begin());
    uint8_t* str = &result[0];
    size_t j = 0, pad = data.size_bytes() % 3;
    const size_t last = data.size_bytes() - pad;

    for (size_t i = 0; i < last; i += 3) {
        int n = int(p[i]) << 16 | int(p[i + 1]) << 8 | p[i + 2];
        str[j++] = detail::B64chars[n >> 18];
        str[j++] = detail::B64chars[n >> 12 & 0x3F];
        str[j++] = detail::B64chars[n >> 6 & 0x3F];
        str[j++] = detail::B64chars[n & 0x3F];
    }
    if (pad) /// set padding
    {
        int n = --pad ? int(p[last]) << 8 | p[last + 1] : p[last];
        str[j++] = detail::B64chars[pad ? n >> 10 & 0x3F : n >> 2];
        str[j++] = detail::B64chars[pad ? n >> 4 & 0x03F : n << 4 & 0x3F];
        str[j++] = pad ? detail::B64chars[n << 2 & 0x3F] : '=';
    }
    return result;
}

inline std::vector<uint8_t> base64decode(span<const uint8_t> data) {
    if (data.empty())
        return {};

    auto len = data.size_bytes();
    auto p = data.data();
    size_t j = 0, pad1 = len % 4 || p[len - 1] == '=',
           pad2 = pad1 && (len % 4 > 2 || p[len - 2] != '=');
    const size_t last = (len - pad1) / 4 << 2;
    std::vector<uint8_t> result(last / 4 * 3 + pad1 + pad2, '\0');
    auto str = &result[0];

    for (size_t i = 0; i < last; i += 4) {
        int n = detail::B64index[p[i]] << 18 | detail::B64index[p[i + 1]] << 12 |
                detail::B64index[p[i + 2]] << 6 | detail::B64index[p[i + 3]];
        str[j++] = n >> 16;
        str[j++] = n >> 8 & 0xFF;
        str[j++] = n & 0xFF;
    }
    if (pad1) {
        int n = detail::B64index[p[last]] << 18 | detail::B64index[p[last + 1]] << 12;
        str[j++] = n >> 16;
        if (pad2) {
            n |= detail::B64index[p[last + 2]] << 6;
            str[j++] = n >> 8 & 0xFF;
        }
    }
    return result;
}
} // namespace tos