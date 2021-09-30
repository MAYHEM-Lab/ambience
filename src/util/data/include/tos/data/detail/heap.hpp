#pragma once

#include <iterator>
#include <utility>

namespace tos::data::heap {
namespace detail {
template<class RandomIter, class Comparator>
RandomIter
heap_down(RandomIter begin, RandomIter end, const Comparator& cmp, int idx, int len) {
    auto lchild = idx * 2 + 1;
    auto rchild = idx * 2 + 2;

    if (len <= 1 || lchild >= len) {
        return begin + idx;
    }

    auto swap_idx = lchild;
    if (rchild < len) {
        auto& left = *(begin + lchild);
        auto& right = *(begin + rchild);

        if (cmp(right, left)) {
            ++swap_idx;
        }
    }

    if (cmp(*(begin + swap_idx), *(begin + idx))) {
        using std::swap;
        swap(*(begin + idx), *(begin + swap_idx));
        return heap_down(begin, end, cmp, swap_idx, len);
    }

    return begin + idx;
}

template<class RandomIter, class Comparator>
RandomIter heap_up(RandomIter begin, RandomIter end, const Comparator& cmp, int idx) {
    if (idx == 0) {
        return begin;
    }

    auto parent = (idx - 1) / 2;

    if (cmp(*(begin + idx), *(begin + parent))) {
        using std::swap;
        swap(*(begin + idx), *(begin + parent));
    }

    return heap_up(begin, end, cmp, parent);
}

struct empty_value {};

template<class KeyType, class ValueType, class CompareT>
struct heap_elem_type {
    using type = std::pair<KeyType, ValueType>;

    static constexpr auto compare(const type& left, const type& right) {
        return CompareT{}(left.first, right.first);
    }

    static void set_key(type& elem, KeyType&& key) {
        elem.first = std::move(key);
    }
};

template<class KeyType, class CompareT>
struct heap_elem_type<KeyType, empty_value, CompareT> {
    using type = KeyType;

    static constexpr auto compare(const KeyType& left, const KeyType& right) {
        return CompareT{}(left, right);
    }

    static void set_key(type& elem, KeyType&& key) {
        elem = std::move(key);
    }
};
} // namespace detail

template<class RandomIter, class Comparator>
RandomIter push_heap(RandomIter begin, RandomIter end, const Comparator& cmp) {
    return detail::heap_up(begin, end, cmp, std::distance(begin, end) - 1);
}

template<class RandomIter, class Comparator>
RandomIter pop_heap(RandomIter begin, RandomIter end, const Comparator& cmp) {
    return detail::heap_down(begin, end, cmp, 0, std::distance(begin, end));
}
} // namespace tos::data::heap