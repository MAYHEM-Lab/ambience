//
// Created by fatih on 6/27/18.
//

#pragma once

#include <cstddef>
#include <memory>
#include <new>
#include <tos/interrupt.hpp>
#include <tos/ring_buf.hpp>

namespace tos {
template<class T, size_t Len, class RingBufT>
class basic_fixed_fifo {
public:
    void push_isr(T t);
    void push(T t);

    T pop();

    size_t size() const {
        return m_rb.size();
    }

    size_t capacity() const {
        return Len;
    }

    bool empty() const {
        return m_rb.empty();
    }

private:
    union elem {
        elem() {
        }
        ~elem() {
        }
        T t;
    };
    elem m_buf[Len];
    RingBufT m_rb{Len};
};
} // namespace tos

// IMPL

namespace tos {
template<class T, size_t Len, class RingBufT>
void basic_fixed_fifo<T, Len, RingBufT>::push_isr(T t) {
    auto i = m_rb.push();
    new (&m_buf[i].t) T(std::move(t));
}

template<class T, size_t Len, class RingBufT>
void basic_fixed_fifo<T, Len, RingBufT>::push(T t) {
    auto i = m_rb.push();
    // push can block, so we disable interrupts after we get the index
    int_guard ig;
    new (&m_buf[i].t) T(std::move(t));
}

template<class T, size_t Len, class RingBufT>
T basic_fixed_fifo<T, Len, RingBufT>::pop() {
    auto i = m_rb.pop();
    // pop can block, so we disable interrupts after we get the index
    int_guard ig;
    auto ptr = &(m_buf[i].t);
    auto res = std::move(*ptr);
    std::destroy_at(ptr);
    return res;
}
} // namespace tos
