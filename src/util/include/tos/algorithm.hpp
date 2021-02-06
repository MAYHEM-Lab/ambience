#pragma once

#include <algorithm>
#include <cstddef>
#include <functional>
#include <iterator>

namespace tos {
template<class Iter, class Fn>
Iter consecutive_find_if(Iter begin, Iter end, std::size_t n, Fn&& predicate) {
    auto cur = std::find_if(begin, end, predicate);
    if (cur == end) {
        return end;
    }

    auto mark = cur++;
    for (size_t i = 1; i < n; ++i, ++cur) {
        if (cur == end) {
            return end;
        }

        if (!predicate(*cur)) {
            return consecutive_find_if(cur + 1, end, n, std::forward<Fn>(predicate));
        }
    }

    return mark;
}

template<class Iter>
Iter consecutive_find(Iter first, Iter last, std::size_t n) {
    return consecutive_find_if(first, last, n, std::equal_to<decltype(*first)>{});
}
} // namespace tos