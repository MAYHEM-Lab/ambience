//
// Created by fatih on 6/22/19.
//

#pragma once

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <tos/compiler.hpp>
#include <tos/span.hpp>
#include <utility>
#include <tos/hex.hpp>

namespace tos {
struct uuid {
    uint8_t id_[16];

    static constexpr uuid from_canonical_string(tos::span<const char> canonical) {
        using detail::byte_from_hex;
        // Canonical string format:
        // 00112233-4455-6677-8899-aabbccddeeff
        uuid res{};
        res.id_[15] = byte_from_hex(canonical.slice(0, 2));
        res.id_[14] = byte_from_hex(canonical.slice(2, 2));
        res.id_[13] = byte_from_hex(canonical.slice(4, 2));
        res.id_[12] = byte_from_hex(canonical.slice(6, 2));
        res.id_[11] = byte_from_hex(canonical.slice(9, 2));
        res.id_[10] = byte_from_hex(canonical.slice(11, 2));
        res.id_[9] = byte_from_hex(canonical.slice(14, 2));
        res.id_[8] = byte_from_hex(canonical.slice(16, 2));
        res.id_[7] = byte_from_hex(canonical.slice(19, 2));
        res.id_[6] = byte_from_hex(canonical.slice(21, 2));
        res.id_[5] = byte_from_hex(canonical.slice(24, 2));
        res.id_[4] = byte_from_hex(canonical.slice(26, 2));
        res.id_[3] = byte_from_hex(canonical.slice(28, 2));
        res.id_[2] = byte_from_hex(canonical.slice(30, 2));
        res.id_[1] = byte_from_hex(canonical.slice(32, 2));
        res.id_[0] = byte_from_hex(canonical.slice(34, 2));

        return res;
    }
};

inline bool operator==(const uuid& a, const uuid& b) {
    return std::equal(std::begin(a.id_), std::end(a.id_), std::begin(b.id_));
}

inline bool operator!=(const uuid& a, const uuid& b) {
    return !(a == b);
}
} // namespace tos
