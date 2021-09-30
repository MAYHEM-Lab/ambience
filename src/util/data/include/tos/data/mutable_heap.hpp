#pragma once

#include <tos/data/detail/heap.hpp>
#include <tos/data/detail/vector_storage.hpp>
#include <tos/intrusive_list.hpp>
#include <tos/late_constructed.hpp>

namespace tos::data::heap {
template<class Base>
struct tracker;

template<class Base>
struct trackable;

template<class Base>
void swap(trackable<Base>& left, trackable<Base>& right);

template<class Base>
struct trackable {
    using base_type = Base;
    base_type value;

    trackable(base_type b)
        : value(std::move(b)) {
    }

    operator base_type&() {
        return value;
    }

    tracker<Base>* trak;

    friend void swap<>(trackable& left, trackable& right);
};

template<class Base>
struct tracker {
    tracker()
        : m_node{} {
    }

    void bind(trackable<Base>& t) {
        m_ptr = &t;
    }

    union {
        trackable<Base>* m_ptr;
        list_node<tracker> m_node;
    };

    friend void swap(tracker& left, tracker& right) {
        std::swap(left.m_ptr, right.m_ptr);
    }
};

template<class Base>
void swap(trackable<Base>& left, trackable<Base>& right) {
    using std::swap;
    swap(static_cast<Base&>(left), static_cast<Base&>(right));
    swap(left.trak, right.trak);

    left.trak->bind(left);
    right.trak->bind(right);
}

template<class KeyType,
         class ValueType = detail::empty_value,
         class Compare = std::less_equal<>,
         template<class> class StorageT = vector_storage>
struct mut_heap {
public:
    using element_type =
        typename detail::heap_elem_type<KeyType, ValueType, Compare>::type;

    template<class... StorageArgs>
    constexpr explicit mut_heap(StorageArgs&&... args)
        : m_store{std::forward<StorageArgs>(args)...}
        , m_track{std::forward<StorageArgs>(args)...} {
        for (auto& tracker : m_track.elements()) {
            m_free_trackers.push_back(tracker);
        }
    }

    constexpr tracker<element_type>* push(element_type value) {
        if (raw_elements().size() <= m_size) {
            return nullptr;
        }
        return &push_unchecked(std::move(value));
    }

    constexpr tracker<element_type>& push_unchecked(element_type&& value) {
        raw_elements()[m_size++].emplace(std::move(value));

        auto t = &m_free_trackers.front();
        m_free_trackers.pop_front();
        elements().back().trak = t;
        t->bind(elements().back());

        tos::data::heap::push_heap(
            elements().begin(),
            elements().end(),
            &detail::heap_elem_type<KeyType, ValueType, Compare>::compare);

        return *t;
    }

    constexpr const element_type& front() const {
        return elements().front();
    }

    constexpr element_type& front() {
        return elements().front();
    }

    constexpr void pop() {
        using std::swap;
        m_free_trackers.push_front(*elements().front().trak);
        swap(elements().front(), elements().back());
        raw_elements()[--m_size].destroy();
        if (empty())
            return;
        tos::data::heap::pop_heap(
            elements().begin(),
            elements().end(),
            &detail::heap_elem_type<KeyType, ValueType, Compare>::compare);
    }

    void decrease(tracker<element_type>& handle, KeyType key) {
        detail::heap_elem_type<KeyType, ValueType, Compare>::set_key(handle.m_ptr->value,
                                                                     std::move(key));
        tos::data::heap::push_heap(
            elements().begin(),
            elements().begin() + std::distance(elements().begin(), handle.m_ptr) + 1,
            &detail::heap_elem_type<KeyType, ValueType, Compare>::compare);
    }

    void increase(tracker<element_type>& handle, KeyType key) {
        detail::heap_elem_type<KeyType, ValueType, Compare>::set_key(handle.m_ptr->value,
                                                                     std::move(key));
        tos::data::heap::detail::heap_down(
            elements().begin(),
            elements().end(),
            &detail::heap_elem_type<KeyType, ValueType, Compare>::compare,
            std::distance(elements().begin(), handle.m_ptr),
            elements().size());
    }

    [[nodiscard]] constexpr size_t size() const {
        return m_size;
    }

    [[nodiscard]] constexpr bool empty() const {
        return m_size == 0;
    }

    ~mut_heap() {
        for (auto& elem : raw_elements().slice(0, m_size)) {
            elem.destroy();
        }
    }

private:
    using inner_element_type = trackable<element_type>;
    constexpr span<maybe_constructed<inner_element_type>> raw_elements() {
        return m_store.elements();
    }

    span<inner_element_type> elements() {
        return span<inner_element_type>(
            reinterpret_cast<inner_element_type*>(raw_elements().data()), m_size);
    }

    int m_size = 0;
    StorageT<maybe_constructed<inner_element_type>> m_store;
    StorageT<tracker<element_type>> m_track;
    intrusive_list<tracker<element_type>, through_member<&tracker<element_type>::m_node>>
        m_free_trackers;
};
} // namespace tos::data::heap