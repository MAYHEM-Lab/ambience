#pragma once

#include <cstdint>
#include <atomic>
#include <algorithm>

namespace tos::trace {
struct basic_counter {
    constexpr void inc(int by = 1) {
        m_count += by;
    }

    constexpr void max(uint64_t sample) {
        m_count = std::max<uint64_t>(sample, m_count);
    }

    uint64_t get() const {
        return m_count;
    }

private:
    uint64_t m_count = 0;
};

struct basic_atomic_counter {
    void inc(int by = 1) {
        m_count.fetch_add(1, std::memory_order_relaxed);
    }

    uint64_t get() const {
        return m_count.load(std::memory_order_relaxed);
    }

private:
    std::atomic<uint64_t> m_count = 0;
};
}