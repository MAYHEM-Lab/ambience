#pragma once

#include <tos/late_constructed.hpp>
#include <tos/span.hpp>
#include <utility>
#include <vector>

namespace tos::data::heap {
struct empty_value {};

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

template<class RandomIter, class Comparator>
RandomIter push_heap(RandomIter begin, RandomIter end, const Comparator& cmp) {
    return heap_up(begin, end, cmp, std::distance(begin, end) - 1);
}

template<class RandomIter, class Comparator>
RandomIter pop_heap(RandomIter begin, RandomIter end, const Comparator& cmp) {
    return heap_down(begin, end, cmp, 0, std::distance(begin, end));
}
} // namespace detail

template<class Type>
struct vector_storage {
    explicit vector_storage(int size)
        : m_elements(size) {
    }

    std::vector<maybe_constructed<Type>> m_elements;

    constexpr span<maybe_constructed<Type>> raw_elements() {
        return m_elements;
    }
};

template<class KeyType, class ValueType, class CompareT>
struct heap_elem_type {
    using type = std::pair<KeyType, ValueType>;

    static constexpr auto compare(const type& left, const type& right) {
        return CompareT{}(left.first, right.first);
    }
};

template<class KeyType, class CompareT>
struct heap_elem_type<KeyType, empty_value, CompareT> {
    using type = KeyType;

    static constexpr auto compare(const KeyType& left, const KeyType& right) {
        return CompareT{}(left, right);
    }
};

template<class KeyType,
         class ValueType = empty_value,
         class Compare = std::less_equal<>,
         class StorageT =
             vector_storage<typename heap_elem_type<KeyType, ValueType, Compare>::type>>
struct heap {
    using element_type = typename heap_elem_type<KeyType, ValueType, Compare>::type;

    template<class... StorageArgs>
    constexpr explicit heap(StorageArgs&&... args)
        : m_store{std::forward<StorageArgs>(args)...} {
    }

    constexpr element_type* push(element_type value) {
        if (raw_elements().size() <= m_size) {
            return nullptr;
        }
        return &push_unchecked(std::move(value));
    }

    constexpr element_type& push_unchecked(element_type&& value) {
        raw_elements()[m_size++].emplace(std::move(value));
        return *detail::push_heap(elements().begin(),
                                  elements().end(),
                                  &heap_elem_type<KeyType, ValueType, Compare>::compare);
    }

    constexpr const element_type& front() const {
        return elements().front();
    }

    constexpr element_type& front() {
        return elements().front();
    }

    constexpr void pop() {
        using std::swap;
        swap(elements().front(), elements().back());
        raw_elements()[--m_size].destroy();
        if (empty()) return;
        detail::pop_heap(elements().begin(),
                         elements().end(),
                         &heap_elem_type<KeyType, ValueType, Compare>::compare);
    }

    [[nodiscard]] constexpr size_t size() const {
        return m_size;
    }

    [[nodiscard]] constexpr bool empty() const {
        return m_size == 0;
    }

    ~heap() {
        for (auto& elem : raw_elements().slice(0, m_size)) {
            elem.destroy();
        }
    }

private:
    constexpr span<maybe_constructed<element_type>> raw_elements() {
        return m_store.raw_elements();
    }

    span<element_type> elements() {
        return span<element_type>(reinterpret_cast<element_type*>(raw_elements().data()),
                                  m_size);
    }

    StorageT m_store{};
    size_t m_size = 0;
};
} // namespace tos::data::heap