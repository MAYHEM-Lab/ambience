#pragma once

#include <algorithm>
#include <array>
#include <cstdint>
#include <numeric>

namespace tos {
constexpr uint64_t bitmask(int len) {
    uint64_t res = 0;
    for (int i = 0; i < len; ++i) {
        res <<= 1;
        res |= 1;
    }
    return res;
}

template<size_t N>
constexpr auto compute_level_bit_begins(const std::array<int, N>& level_bits) {
    std::array<int, N> inv_bits = level_bits;
    std::reverse(inv_bits.begin(), inv_bits.end());

    std::array<int, N + 1> shifted_bits = {0};
    std::copy(inv_bits.begin(), inv_bits.end(), shifted_bits.begin() + 1);

    std::array<int, N> total_bits{};
    std::partial_sum(shifted_bits.begin(), shifted_bits.end() - 1, total_bits.begin());

    std::reverse(total_bits.begin(), total_bits.end());

    return total_bits;
}

template<size_t N>
constexpr auto compute_level_bit_sums(const std::array<int, N>& level_bits) {
    std::array<int, N> inv_bits = level_bits;
    std::reverse(inv_bits.begin(), inv_bits.end());

    std::array<int, N> total_bits{};
    std::partial_sum(inv_bits.begin(), inv_bits.end(), total_bits.begin());

    std::reverse(total_bits.begin(), total_bits.end());

    return total_bits;
}

template<size_t N>
constexpr auto compute_level_masks(const std::array<int, N>& level_bits) {
    std::array<int, N> inv_bits = level_bits;
    std::reverse(inv_bits.begin(), inv_bits.end());

    std::array<int, N> total_bits{};
    std::partial_sum(inv_bits.begin(), inv_bits.end(), total_bits.begin());

    std::array<uint64_t, N> level_masks{};
    level_masks[0] = bitmask(total_bits[0]);

    for (size_t i = 1; i < level_masks.size(); ++i) {
        level_masks[i] = bitmask(total_bits[i]) ^ bitmask(total_bits[i - 1]);
    }

    std::reverse(level_masks.begin(), level_masks.end());

    return level_masks;
}
} // namespace tos