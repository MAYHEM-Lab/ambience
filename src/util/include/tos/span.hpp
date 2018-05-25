//
// Created by fatih on 5/24/18.
//

#pragma once

#include <stddef.h>

namespace tos
{
    template <class T>
    class span
    {
    public:
        span(T* base, size_t len) : m_base(base), m_len(len) {}

        template <size_t Sz>
        span(T (&arr)[Sz]) : m_base(arr), m_len(Sz) {}

        size_t size() const { return m_len; }

        T* data() { return m_base; }
        const T* data() const { return m_base; }

    private:
        T* m_base;
        size_t m_len;
    };
}