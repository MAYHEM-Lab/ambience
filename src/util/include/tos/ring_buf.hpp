//
// Created by Mehmet Fatih BAKIR on 29/04/2018.
//

#pragma once

#include <tos/semaphore.hpp>
#include <tos/mutex.hpp>

namespace tos
{
    template <class T>
    class base_ring_buf
    {
    public:
        explicit base_ring_buf(size_t sz)
                : m_cap{sz} {}

        size_t capacity() const
        {
            return m_cap;
        }

        size_t translate(size_t index) const
        {
            return (m_begin + index) % m_cap;
        }

    protected:
        size_t push()
        {
            return (m_begin + get_size()) % m_cap;
        }

        size_t pop()
        {
            auto res = m_begin;
            m_begin = (m_begin + 1) % m_cap;
            return res;
        }

    private:
        size_t get_size() const
        {
            return static_cast<const T*>(this)->size();
        }

        size_t m_begin = 0;
        size_t m_cap;
    };

    class sync_ring_buf
            : public base_ring_buf<sync_ring_buf>
    {
    public:
        explicit sync_ring_buf(size_t cap) : base_ring_buf{cap}, m_read{0}, m_put{cap} {}

        size_t size() const
        {
            int8_t sz = get_count(m_read);
            return sz < 0 ? 0 : sz;
        }

        using base_ring_buf::capacity;
        using base_ring_buf::translate;

        size_t push()
        {
            m_put.down();
            auto res = base_ring_buf::push();
            m_read.up();
            return res;
        }

        size_t pop()
        {
            m_read.down();
            auto res = base_ring_buf::pop();
            m_put.up();
            return res;
        }

    private:
        tos::semaphore m_read, m_put;
    };

    class ring_buf
            : public base_ring_buf<ring_buf>
    {
    public:
        explicit ring_buf(size_t cap) : base_ring_buf{cap}, m_sz{0} {}

        size_t size() const { return m_sz; }

        using base_ring_buf::capacity;
        using base_ring_buf::translate;

        size_t push()
        {
            auto res = base_ring_buf::push();
            m_sz++;
            return res;
        }

        size_t pop()
        {
            auto res = base_ring_buf::pop();
            m_sz--;
            return res;
        }

    private:
        size_t m_sz;
    };
}
