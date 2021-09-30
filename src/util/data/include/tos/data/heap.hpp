#pragma once

#include <tos/data/detail/heap.hpp>
#include <tos/data/detail/vector_storage.hpp>
#include <tos/late_constructed.hpp>

namespace tos::data::heap {
template<class KeyType,
         class ValueType = detail::empty_value,
         class Compare = std::less_equal<>,
         class StorageT = vector_storage<maybe_constructed<
             typename detail::heap_elem_type<KeyType, ValueType, Compare>::type>>>
struct heap {
    using element_type =
        typename detail::heap_elem_type<KeyType, ValueType, Compare>::type;

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
        return *tos::data::heap::push_heap(
            elements().begin(),
            elements().end(),
            &detail::heap_elem_type<KeyType, ValueType, Compare>::compare);
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
        if (empty())
            return;
        tos::data::heap::pop_heap(elements().begin(),
                 elements().end(),
                 &detail::heap_elem_type<KeyType, ValueType, Compare>::compare);
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
        return m_store.elements();
    }

    span<element_type> elements() {
        return span<element_type>(reinterpret_cast<element_type*>(raw_elements().data()),
                                  m_size);
    }

    StorageT m_store{};
    size_t m_size = 0;
};
} // namespace tos::data::heap