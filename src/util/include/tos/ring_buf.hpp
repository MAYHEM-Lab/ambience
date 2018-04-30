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
        base_ring_buf(void* data, size_t sz, size_t obj_size)
                : m_data{data}, m_cap{sz}, m_read{0}, m_put{int8_t(sz)} {}

        void* get(size_t obj_size, size_t index)
        {
            return (char*)m_data + translate(index) * obj_size;
        }

        const void* get(size_t obj_size, size_t index) const
        {
            return (char*)m_data + translate(index) * obj_size;
        }

        size_t put(size_t obj_size)
        {
            tos::lock_guard<tos::mutex> lk{m_mtx};
            m_put.down();
            auto res = m_end;
            m_end = (m_end + 1) % m_cap;
            m_read.up();
            return res;
        }

        void pop()
        {
            tos::lock_guard<tos::mutex> lk{m_mtx};

            m_read.down();

            m_begin = (m_begin + 1) % m_cap;

            m_put.up();
        }

        size_t capacity() const {
            return m_cap;
        }

    private:

        size_t translate(size_t index) const
        {
            return (m_begin + index) % m_cap;
        }

        size_t m_begin = 0, m_end = 0;
        tos::mutex m_mtx;
        tos::semaphore m_read, m_put;
        size_t m_cap;
        void* m_data;
    };

    template <class T>
    class ring_buf
    {
    public:
        explicit ring_buf(T* data, size_t sz)
                : m_data{data}, m_put{int8_t(sz)}, m_read{0}, m_cap{sz} {}

        T& get(size_t index)
        {
            return m_data[m_head];
        }

        const T& get(size_t index) const
        {
            return m_data[m_head];
        }

        void pop()
        {
            m_read.down();
            at(m_tail)->~T();
            m_put.up();
        }

        template <class U>
        void push(U&& u)
        {
            m_put.down();
        }

        T& operator[](size_t index) { return get(index); }
        const T& operator[](size_t index) const { return get(index); }

    private:

        T* at(size_t index)
        {
            return reinterpret_cast<T*>(m_data + m_tail);
        }

        size_t m_head = 0, m_tail = 0;
        tos::semaphore m_read, m_put;
        size_t m_cap;
        char* m_data;
    };
}
