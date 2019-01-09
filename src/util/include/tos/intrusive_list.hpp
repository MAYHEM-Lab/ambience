#pragma once

#include <stdint.h>
#include <stddef.h>
#include <iterator>
#include <utility>
#include <tos/utility.hpp>

namespace tos
{
    template <class T> class intrusive_list;
    template <class T> class intrusive_list_iterator;

    template <class T>
    class list_node : public non_copy_movable
    {
        friend class intrusive_list<T>;
        friend class intrusive_list_iterator<T>;

    public:
        T* prev;
        T* next;
    };

    template <class T>
    class intrusive_list_iterator
        : public std::iterator<std::bidirectional_iterator_tag, T>
    {
    public:
        T&operator*();
        T*operator->();
        const intrusive_list_iterator operator++(int);
        intrusive_list_iterator&operator++();
        const intrusive_list_iterator operator--(int);
        intrusive_list_iterator&operator--();

        bool operator!=(const intrusive_list_iterator&);

        bool operator==(const intrusive_list_iterator& rhs)
        {
            return !(*this != rhs);
        }
    private:
        friend class intrusive_list<T>;
        explicit intrusive_list_iterator(T* p) : m_curr(p) {}
        T* m_curr;
    };

    template<class T>
    T &intrusive_list_iterator<T>::operator*() {
        return *m_curr;
    }

    template<class T>
    T *intrusive_list_iterator<T>::operator->() {
        return m_curr;
    }

    template<class T>
    const intrusive_list_iterator<T> intrusive_list_iterator<T>::operator++(int) {
        auto ret = *this;
        ++(*this);
        return ret;
    }

    template<class T>
    intrusive_list_iterator<T> &intrusive_list_iterator<T>::operator++() {
        m_curr = m_curr->next;
        return *this;
    }

    template<class T>
    const intrusive_list_iterator<T> intrusive_list_iterator<T>::operator--(int) {
        auto ret = *this;
        --(*this);
        return ret;
    }

    template<class T>
    intrusive_list_iterator<T> &intrusive_list_iterator<T>::operator--() {
        m_curr = m_curr->prev;
        return *this;
    }

    /**
     * This class represents a non-owning doubly linked list
     *
     * Objects that are stored in an intrusive_list are not owned
     * by the list and thus must be allocated and deallocated by
     * the user.
     *
     * @tparam T type of the objects to be stored in the list
     */
    template <class T>
    class intrusive_list
    {
        T* m_head;
        T* m_tail;

    public:

        /**
         * Constructs an empty list
         */
        intrusive_list() : m_head(nullptr), m_tail(nullptr) {}

        intrusive_list(intrusive_list&& rhs) noexcept
            : m_head{std::exchange(rhs.m_head, nullptr)}
            , m_tail{std::exchange(rhs.m_tail, nullptr)}
        {
        }

        intrusive_list(const intrusive_list&) = delete;
        ~intrusive_list() = default;

        intrusive_list& operator=(intrusive_list&& rhs) noexcept
        {
            m_head = std::exchange(rhs.m_head, nullptr);
            m_tail = std::exchange(rhs.m_tail, nullptr);
            return *this;
        }

        intrusive_list&operator=(const intrusive_list&) = delete;

        using iterator_t = intrusive_list_iterator<T>;

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
        bool empty() const { return m_head == nullptr; }

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
    };

    template<class T>
    bool intrusive_list_iterator<T>::operator!=(const intrusive_list_iterator & rhs) {
        return m_curr != rhs.m_curr;
    }

    template<class T>
    auto intrusive_list<T>::begin() const -> iterator_t {
        return iterator_t{m_head};
    }

    template<class T>
    auto intrusive_list<T>::end() const -> iterator_t {
        return iterator_t{nullptr};
    }

    template<class T>
    auto intrusive_list<T>::push_back(T &t) -> iterator_t
    {
        if (empty())
        {
            m_head = &t;
            t.prev = nullptr;
        } else {
            t.prev = m_tail;
            m_tail->next = &t;
        }
        m_tail = &t;
        t.next = nullptr;
        return iterator_t{ &t };
    }

    template <class T>
    auto intrusive_list<T>::push_front(T& t) -> iterator_t
    {
        if (empty())
        {
            m_tail = &t;
            t.next = nullptr;
        } else {
            t.next = m_head;
            m_head->prev = &t;
        }
        m_head = &t;
        t.prev = nullptr;
        return iterator_t{ &t };
    }

    template<class T>
    auto intrusive_list<T>::insert(intrusive_list::iterator_t at, T& t) -> iterator_t
    {
        if (at == begin())
        {
            push_front(t);
            return iterator_t{ &t };
        }
        else if (at == end())
        {
            return push_back(t);
        }

        t.prev = at->prev;
        at->prev = &t;
        t.next = &(*at);

        if (t.prev)
        {
            t.prev->next = &t;
        }

        return iterator_t{ &t };
    }

    template<class T>
    T &intrusive_list<T>::front() {
        return *m_head;
    }

    template<class T>
    T &intrusive_list<T>::back() {
        return *m_tail;
    }

    template<class T>
    const T &intrusive_list<T>::front() const {
        return *m_head;
    }

    template<class T>
    const T &intrusive_list<T>::back() const {
        return *m_tail;
    }

    template<class T>
    void intrusive_list<T>::clear() {
        m_head = nullptr;
        m_tail = nullptr;
    }

    template<class T>
    void intrusive_list<T>::pop_back() {
        if (m_head == m_tail)
        {
            clear();
            return;
        }
        auto tail = m_tail;
        m_tail = tail->prev;
        m_tail->next = nullptr;
    }

    template<class T>
    void intrusive_list<T>::pop_front() {
        if (m_head == m_tail)
        {
            clear();
            return;
        }
        auto head = m_head;
        m_head = head->next;
        m_head->prev = nullptr;
    }

    template<class T>
    auto intrusive_list<T>::erase(iterator_t it) -> iterator_t
    {
        auto ptr = it.m_curr;

        if (m_head == m_tail)
        {
            //assert(m_head == ptr);
            m_head = nullptr;
            m_tail = nullptr;
        }
        else if (m_head == ptr)
        {
            m_head = ptr->next;
            m_head->prev = nullptr;
        }
        else if (m_tail == ptr)
        {
            m_tail = ptr->prev;
            m_tail->next = nullptr;
        }
        else
        {
            ptr->prev->next = ptr->next;
            ptr->next->prev = ptr->prev;
        }

        return iterator_t{ptr->next};
    }
} // namespace tos
