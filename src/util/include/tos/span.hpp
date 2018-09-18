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
        using iterator = T*;

        constexpr span(T* base, size_t len) : m_base(base), m_len(len) {}

        constexpr span(T* base, T* end) : m_base{base}, m_len{end - base} {}

        template <size_t Sz>
        constexpr span(T (&arr)[Sz]) : m_base(arr), m_len(Sz) {}

        constexpr size_t size() const { return m_len; }

        constexpr T* data() { return m_base; }
        constexpr const T* data() const { return m_base; }

        constexpr T& operator[](size_t ind) {
            return m_base[ind];
        }

        constexpr const T& operator[](size_t ind) const {
            return m_base[ind];
        }

        constexpr T* begin() { return m_base; }
        constexpr T* end() { return m_base + m_len; }

        constexpr const T* begin() const { return m_base; }
        constexpr const T* end() const { return m_base + m_len; }

        constexpr operator span<const T>() const {
            return { m_base, m_len };
        }

        constexpr span slice(size_t begin, size_t len){
            return { m_base + begin, len };
        }

    private:
        T* m_base;
        ptrdiff_t m_len;
    };
}