#pragma once

#include <cstdint>

namespace tos::trace {

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