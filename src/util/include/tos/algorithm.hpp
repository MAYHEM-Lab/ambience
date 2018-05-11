//
// Created by Mehmet Fatih BAKIR on 10/05/2018.
//

#pragma once

namespace tos
{
    template <class T>
    constexpr T max(const T &a, const T &b) {
        return a < b ? b : a;
    }

    template <class T>
    constexpr T min(const T &a, const T &b) {
        return a > b ? b : a;
    }

    template <class ItT>
    constexpr auto max_range(ItT begin, ItT end) -> remove_reference_t<decltype(*begin)>
    {
        if (begin + 1 == end)
        {
            return *begin;
        }
        else
        {
            auto mid = begin + (end - begin) / 2;
            return max(
                max_range(begin, mid),
                max_range(mid, end)
            );
        }
    }
}
