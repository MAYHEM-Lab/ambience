#pragma once

#include <iterator>
#include <stddef.h>
#include <stdint.h>
#include <tos/meta/offsetof.hpp>
#include <tos/utility.hpp>
#include <utility>

namespace tos {
template<class T, class Access>
class intrusive_list;

template<class T, class Access>
class intrusive_list_iterator;

template<class T>
class list_node : public non_copy_movable {
public:
    list_node<T>* prev; // NOLINT
    list_node<T>* next; // NOLINT
};

struct through_base {
    template<class T, class U>
    static list_node<T>& access(U& u) {
        static_assert(std::is_base_of_v<list_node<T>, U>);
        return static_cast<list_node<T>&>(u);
    }

    template<class T, class U>
    static const list_node<T>& access(const U& u) {
        static_assert(std::is_base_of_v<list_node<T>, U>);
        return static_cast<const list_node<T>&>(u);
    }

    template<class T, class U>
    static T& reverse(list_node<U>& elem) {
        static_assert(std::is_base_of_v<list_node<U>, T>);
        return static_cast<T&>(elem);
    }

    template<class T, class U>
    static const T& reverse(const list_node<U>& elem) {
        static_assert(std::is_base_of_v<list_node<U>, T>);
        return static_cast<const T&>(elem);
    }
};

template<auto Member>
struct through_member {
    template<class T, class U>
    static list_node<T>& access(U& u) {
        return u.*Member;
    }

    template<class T, class U>
    static const list_node<T>& access(const U& u) {
        return u.*Member;
    }

    template<class T, class U>
    static T& reverse(list_node<U>& elem) {
        auto off = meta::offset_of<Member>();
        return *reinterpret_cast<T*>(reinterpret_cast<char*>(&elem) - off);
    }

    template<class T, class U>
    static const T& reverse(const list_node<U>& elem) {
        auto off = meta::offset_of<Member>();
        return *reinterpret_cast<const T*>(reinterpret_cast<const char*>(&elem) - off);
    }

private:
};

template<class T, class Access>
class intrusive_list_iterator : public std::iterator<std::bidirectional_iterator_tag, T> {
public:
    T& operator*() {
        return Access::template reverse<T>(*m_curr);
    }
    T* operator->() {
        return &Access::template reverse<T>(*m_curr);
    }
    const intrusive_list_iterator operator++(int);
    intrusive_list_iterator& operator++();
    const intrusive_list_iterator operator--(int);
    intrusive_list_iterator& operator--();

    bool operator!=(const intrusive_list_iterator&) const;

    bool operator==(const intrusive_list_iterator& rhs) const {
        return !(*this != rhs);
    }

private:
    friend class intrusive_list<T, Access>;
    template<class AccessT, class ElemT>
    friend intrusive_list_iterator<ElemT, AccessT>
    ad_hoc_list_iter(list_node<ElemT>& elem);
    explicit intrusive_list_iterator(list_node<T>* p)
        : m_curr(p) {
    }
    list_node<T>* m_curr;
};

/**
 * This class represents a non-owning doubly linked list
 *
 * Objects that are stored in an intrusive_list are not owned
 * by the list and thus must be allocated and deallocated by
 * the user.
 *
 * @tparam T type of the objects to be stored in the list
 */
template<class T, class Access = through_base>
class intrusive_list {
    list_node<T>* m_head;
    list_node<T>* m_tail;

public:
    /**
     * Constructs an empty list
     */
    intrusive_list()
        : m_head(nullptr)
        , m_tail(nullptr) {
    }

    intrusive_list(intrusive_list&& rhs) noexcept
        : m_head{std::exchange(rhs.m_head, nullptr)}
        , m_tail{std::exchange(rhs.m_tail, nullptr)} {
    }

    intrusive_list(const intrusive_list&) = delete;
    ~intrusive_list() = default;

    intrusive_list& operator=(intrusive_list&& rhs) noexcept {
        m_head = std::exchange(rhs.m_head, nullptr);
        m_tail = std::exchange(rhs.m_tail, nullptr);
        return *this;
    }

    intrusive_list& operator=(const intrusive_list&) = delete;

    using iterator_t = intrusive_list_iterator<T, Access>;

    /**
     * Returns the number of elements in the list
     * Traverses the whole list, prefer `empty` if possible
     *
     * Complexity: O(N)
     *
     * @return number of elements
     */
    size_t size() const {
        return std::distance(begin(), end());
    }

    /**
     * Returns whether the list is empty or not
     * @return list contains any elements
     */
    bool empty() const {
        return m_head == nullptr;
    }

    /**
     * Inserts a new element at the end of the list
     *
     * Calling this function with an object that's already in a list
     * causes undefined behaviour
     *
     * Complexity: O(1)
     *
     * @param t object to insert
     * @return iterator to the object
     */
    iterator_t push_back(T& t);

    /**
     * Inserts a new element at the beginning of the list
     *
     * Calling this function with an object that's already in a list
     * causes undefined behaviour
     *
     * Complexity: O(1)
     *
     * @param t object to insert
     * @return iterator to the object
     */
    iterator_t push_front(T& t);

    /**
     * Returns a reference to the first element of the list
     *
     * Calling this function on an empty list is undefined
     *
     * @return reference to the first element
     */
    T& front();

    /**
     * Returns a reference to the last element of the list
     *
     * Calling this function on an empty list is undefined
     *
     * @return reference to the last element
     */
    T& back();

    /**
     * Returns a reference to the first element of the list
     *
     * Calling this function on an empty list is undefined
     *
     * @return reference to the first element
     */
    const T& front() const;

    /**
     * Returns a reference to the last element of the list
     *
     * Calling this function on an empty list is undefined
     *
     * @return reference to the last element
     */
    const T& back() const;

    /**
     * Inserts a new object to the given location
     *
     * The object will be inserted in the place of *at, this means
     * when the function returns, the returned iterator's next will
     * point to *at
     *
     * Calling this function with an object that's already in a list
     * causes undefined behaviour
     *
     * Complexity: O(1)
     *
     * @param at location to insert the object to
     * @param t object to insert
     * @return iterator to the inserted object
     */
    iterator_t insert(iterator_t at, T& t);

    /**
     * Removes the element at the end of the list
     *
     * Complexity: O(1)
     *
     * Calling this function on an empty list is undefined
     */
    void pop_back();

    /**
     * Removes the element at the beginning of the list
     *
     * Complexity: O(1)
     *
     * Calling this function on an empty list is undefined
     */
    void pop_front();

    /**
     * Removes all elements from the container
     *
     * Complexity: O(1)
     */
    void clear();

    /**
     * Removes the object pointed by the given iterator from the list
     *
     * Complexity: O(1)
     *
     * @param it iterator to the object to be removed
     * @return iterator to the next element in the list
     */
    iterator_t erase(iterator_t it);

    /**
     * Returns an iterator to the beginning of the list
     * @return the begin iterator
     */
    iterator_t begin() const;

    /**
     * Returns an iterator to one past the last element of the list
     * @return the end iterator
     */
    iterator_t end() const;

    iterator_t unsafe_find(T& t) const {
        return iterator_t{&Access::template access<T>(t)};
    }

    template<class TAccess, class ElemT>
    friend intrusive_list<ElemT, TAccess> ad_hoc_list(list_node<ElemT>& elem);
};

template<class Access = through_base, class ElemT>
intrusive_list<ElemT, Access> ad_hoc_list(list_node<ElemT>& elem) {
    list_node<ElemT>*head, *tail;
    for (head = &elem; head->prev; head = head->prev)
        ;
    for (tail = &elem; tail->next; tail = tail->next)
        ;

    intrusive_list<ElemT, Access> res;
    res.m_head = head;
    res.m_tail = tail;

    return res;
}

template<class Access = through_base, class ElemT>
intrusive_list_iterator<ElemT, Access> ad_hoc_list_iter(list_node<ElemT>& elem) {
    return intrusive_list_iterator<ElemT, Access>(&elem);
}
} // namespace tos

// Implementations

namespace tos {
template<class T, class Access>
auto intrusive_list<T, Access>::begin() const -> iterator_t {
    return iterator_t{m_head};
}

template<class T, class Access>
auto intrusive_list<T, Access>::end() const -> iterator_t {
    return iterator_t{nullptr};
}

template<class T, class Access>
auto intrusive_list<T, Access>::push_back(T& elem) -> iterator_t {
    auto& t = Access::template access<T>(elem);
    if (empty()) {
        m_head = &t;
        t.prev = nullptr;
    } else {
        t.prev = m_tail;
        m_tail->next = &t;
    }
    m_tail = &t;
    t.next = nullptr;
    return iterator_t{&t};
}

template<class T, class Access>
auto intrusive_list<T, Access>::push_front(T& elem) -> iterator_t {
    auto& t = Access::template access<T>(elem);
    if (empty()) {
        m_tail = &t;
        t.next = nullptr;
    } else {
        t.next = m_head;
        m_head->prev = &t;
    }
    m_head = &t;
    t.prev = nullptr;
    return iterator_t{&t};
}

template<class T, class Access>
auto intrusive_list<T, Access>::insert(intrusive_list::iterator_t at, T& elem)
    -> iterator_t {
    auto& t = Access::template access<T>(elem);
    if (at == begin()) {
        push_front(elem);
        return iterator_t{&t};
    } else if (at == end()) {
        return push_back(elem);
    }

    t.prev = at->prev;
    at->prev = &t;
    t.next = &(*at);

    if (t.prev) {
        t.prev->next = &t;
    }

    return iterator_t{&t};
}

template<class T, class Access>
T& intrusive_list<T, Access>::front() {
    return Access::template reverse<T>(*m_head);
}

template<class T, class Access>
T& intrusive_list<T, Access>::back() {
    return Access::template reverse<T>(*m_tail);
}

template<class T, class Access>
const T& intrusive_list<T, Access>::front() const {
    return Access::template reverse<T>(*m_head);
}

template<class T, class Access>
const T& intrusive_list<T, Access>::back() const {
    return Access::template reverse<T>(*m_tail);
}

template<class T, class Access>
void intrusive_list<T, Access>::clear() {
    m_head = nullptr;
    m_tail = nullptr;
}

template<class T, class Access>
void intrusive_list<T, Access>::pop_back() {
    if (m_head == m_tail) {
        clear();
        return;
    }
    auto tail = m_tail;
    m_tail = tail->prev;
    m_tail->next = nullptr;
}

template<class T, class Access>
void intrusive_list<T, Access>::pop_front() {
    if (m_head == m_tail) {
        clear();
        return;
    }
    auto head = m_head;
    m_head = head->next;
    m_head->prev = nullptr;
}

template<class T, class Access>
auto intrusive_list<T, Access>::erase(iterator_t it) -> iterator_t {
    auto ptr = it.m_curr;

    if (m_head == m_tail) {
        // assert(m_head == ptr);
        m_head = nullptr;
        m_tail = nullptr;
    } else if (m_head == ptr) {
        m_head = ptr->next;
        m_head->prev = nullptr;
    } else if (m_tail == ptr) {
        m_tail = ptr->prev;
        m_tail->next = nullptr;
    } else {
        ptr->prev->next = ptr->next;
        ptr->next->prev = ptr->prev;
    }

    return iterator_t{ptr->next};
}

template<class T, class Access>
const intrusive_list_iterator<T, Access>
intrusive_list_iterator<T, Access>::operator++(int) {
    auto ret = *this;
    ++(*this);
    return ret;
}

template<class T, class Access>
intrusive_list_iterator<T, Access>& intrusive_list_iterator<T, Access>::operator++() {
    m_curr = m_curr->next;
    return *this;
}

template<class T, class Access>
const intrusive_list_iterator<T, Access>
intrusive_list_iterator<T, Access>::operator--(int) {
    auto ret = *this;
    --(*this);
    return ret;
}

template<class T, class Access>
intrusive_list_iterator<T, Access>& intrusive_list_iterator<T, Access>::operator--() {
    m_curr = m_curr->prev;
    return *this;
}

template<class T, class Access>
bool intrusive_list_iterator<T, Access>::operator!=(
    const intrusive_list_iterator& rhs) const {
    return m_curr != rhs.m_curr;
}
} // namespace tos