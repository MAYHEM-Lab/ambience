//
// Created by Mehmet Fatih BAKIR on 29/04/2018.
//

#pragma once

#include <tos/semaphore.hpp>
#include <tos/mutex.hpp>

namespace tos
{
    class base_ring_buf
    {
    public:
        explicit base_ring_buf(size_t sz)
                : m_cap{sz}, m_read{0}, m_put{int8_t(sz)} {}

        size_t push()
        {
            m_put.down();
            auto res = (m_begin + size()) % m_cap;
            m_read.up();
            return res;
        }

        void pop()
        {
            m_read.down();
            m_begin = (m_begin + 1) % m_cap;
            m_put.up();
        }

        size_t capacity() const
        {
            return m_cap;
        }

        size_t translate(size_t index) const
        {
            return (m_begin + index) % m_cap;
        }

        size_t size() const
        {
            return get_count(m_read);
        }

    private:
        size_t m_begin = 0;
        tos::semaphore m_read, m_put;
        const size_t m_cap;
    };
}
