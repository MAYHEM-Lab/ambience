//
// Created by Mehmet Fatih BAKIR on 23/10/2018.
//

#pragma once

#include <stddef.h>
#include <tos/span.hpp>
#include <initializer_list>

namespace tos
{
    template <class _Tp, size_t _Sz>
    struct array
    {
        typedef array __self;
        typedef _Tp                                   value_type;
        typedef value_type&                           reference;
        typedef const value_type&                     const_reference;
        typedef value_type*                           iterator;
        typedef const value_type*                     const_iterator;
        typedef value_type*                           pointer;
        typedef const value_type*                     const_pointer;
        typedef size_t                                size_type;
        typedef ptrdiff_t                             difference_type;

        _Tp __m_arr_[_Sz];

        constexpr iterator begin() { return __m_arr_; }
        constexpr iterator end() { return __m_arr_ + _Sz; }

        constexpr const_iterator begin() const { return __m_arr_; }
        constexpr const_iterator end() const { return __m_arr_ + _Sz; }

        constexpr size_type size() const noexcept { return _Sz; }
        constexpr bool empty() const { return false; }

        constexpr reference operator[](size_type __n) {return __m_arr_[__n];}
        constexpr const_reference operator[](size_type __n) const {return __m_arr_[__n];}

        constexpr operator tos::span<_Tp>() { return __m_arr_; }
        constexpr operator tos::span<const _Tp>() const { return __m_arr_; }
    };
}
