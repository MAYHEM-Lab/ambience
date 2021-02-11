//
// Created by Mehmet Fatih BAKIR on 29/04/2018.
//

#pragma once

#include <cstddef>

namespace tos {
template<class T>
class base_ring_buf {
public:
    constexpr explicit base_ring_buf(size_t sz)
        : m_cap{sz} {
    }

    constexpr explicit base_ring_buf(size_t sz, size_t begin)
        : m_begin{begin}
        , m_cap{sz} {
    }

    constexpr ssize_t capacity() const {
        return m_cap;
    }

    constexpr size_t translate(size_t index) const {
        return (m_begin + index) % m_cap;
    }

protected:
    size_t push_base() {
        return (m_begin + get_size()) % m_cap;
    }

    constexpr size_t pop_base() {
        auto res = m_begin;
        m_begin = (m_begin + 1) % m_cap;
        return res;
    }

private:
    size_t get_size() const {
        return static_cast<const T*>(this)->size();
    }

    size_t m_begin = 0;
    size_t m_cap;
};


class ring_buf : public base_ring_buf<ring_buf> {
public:
    /**
     * Constructs a new ring buffer with the given capacity
     * @param cap capacity
     */
    explicit constexpr ring_buf(size_t cap)
        : base_ring_buf{cap}
        , m_sz{0} {
    }

    constexpr ring_buf(size_t cap, size_t sz, size_t begin)
        : base_ring_buf{cap, begin}
        , m_sz{ssize_t(sz)} {
    }

    constexpr ssize_t size() const {
        return m_sz;
    }

    using base_ring_buf::capacity;
    using base_ring_buf::translate;

    size_t push() {
        auto res = push_base();
        m_sz++;
        return res;
    }

    size_t pop() {
        auto res = pop_base();
        m_sz--;
        return res;
    }

    bool empty() const {
        return size() == 0;
    }

private:
    ssize_t m_sz;
};
} // namespace tos
