//
// Created by Mehmet Fatih BAKIR on 10/05/2018.
//

#pragma once

#include <tos/utility.hpp>
#include <tos/span.hpp>

namespace tos
{
    namespace std
    {
        template <class T>
        constexpr T max(const T &a, const T &b) {
            return a < b ? b : a;
        }

        template <class T>
        constexpr T min(const T &a, const T &b) {
            return a > b ? b : a;
        }
    }

    template <class ItT>
    constexpr auto max_range(ItT begin, ItT end) -> std::remove_reference_t<decltype(*begin)>
    {
        if (begin + 1 == end)
        {
            return *begin;
        }
        else
        {
            auto mid = begin + (end - begin) / 2;
            return std::max(
                max_range(begin, mid),
                max_range(mid, end)
            );
        }
    }

    inline int atoi(tos::span<const char> chars)
    {
        int res = 0;
        for (auto p = chars.begin(); p != chars.end() && *p != 0; ++p)
        {
            res = res * 10 + *p - '0';
        }
        return res;
    }

    namespace std
    {
        template<class ForwardIt, class T >
        void fill(ForwardIt first, ForwardIt last, const T& value)
        {
            for (; first != last; ++first) {
                *first = value;
            }
        }

        template<class InputIt, class OutputIt>
        OutputIt copy(InputIt first, InputIt last,
                OutputIt d_first)
        {
            while (first != last) {
                *d_first++ = *first++;
            }
            return d_first;
        }
    }
}
