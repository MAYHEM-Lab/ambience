//
// Created by fatih on 6/28/18.
//

#pragma once

#include <cstdint>
#include <cstddef>
#include <tos/ring_buf.hpp>
#include <tos/semaphore.hpp>
#include <tos/fixed_fifo.hpp>

namespace tos
{
    class sync_ring_buf : private ring_buf
    {
    public:
        explicit sync_ring_buf(size_t cap, size_t sz, size_t begin)
            : ring_buf{cap, sz, begin}, m_read{int16_t(sz)}, m_put{int16_t(cap - sz)} {}
        /**
         * Creates a new synchronized ring buf with the given capacity
         * @param cap capacity
         */
        explicit sync_ring_buf(size_t cap) : ring_buf{cap}, m_read{0}, m_put{int16_t(cap)} {}

        using ring_buf::size;
        using ring_buf::capacity;
        using ring_buf::translate;

        size_t push()
        {
            m_put.down();
            auto res = ring_buf::push();
            m_read.up();
            return res;
        }

        //TODO: return an optional<size_t> here
        size_t push_isr()
        {
            auto cont = try_down_isr(m_put);
            if (!cont) return -1;
            auto res = ring_buf::push();
            m_read.up_isr();
            return res;
        }

        size_t pop()
        {
            m_read.down();
            auto res = ring_buf::pop();
            m_put.up();
            return res;
        }

        bool empty() const {
            return size() == 0;
        }

    private:
        tos::semaphore m_read, m_put;
    };

    template <class ElemT, size_t Size>
    using sync_fixed_fifo = basic_fixed_fifo<ElemT, Size, sync_ring_buf>;
}