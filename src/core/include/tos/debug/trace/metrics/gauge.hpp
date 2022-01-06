#pragma once

#include <cstdint>
#include <array>

namespace tos::trace {
struct basic_min_max_gauge {
    constexpr void set(uint64_t val) {
        cur = val;
        max = std::max(max, cur);
        min = std::min(min, cur);
    }

    constexpr void inc(int by = 1) {
        cur += by;
        max = std::max(max, cur);
    }

    constexpr void dec(int by = 1) {
        cur -= by;
        min = std::min(min, cur);
    }

    std::array<uint64_t, 3> get() const {
        return {min, cur, max};
    }

private:
    uint64_t min = 0;
    uint64_t max = 0;
    uint64_t cur = 0;
};

struct basic_gauge {
    constexpr void set(uint64_t val) {
        m_count = val;
    }

    constexpr void inc(int by = 1) {
        m_count += by;
    }

    constexpr void dec(int by = 1) {
        m_count -= by;
    }

    uint64_t get() const {
        return m_count;
    }

private:
    uint64_t m_count = 0;
};
} // namespace tos::trace