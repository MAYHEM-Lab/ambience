#pragma once

#include <stdint.h>
#include <stddef.h>

namespace tos
{
    template <class T> class intrusive_list;
    template <class T> class intrusive_list_iterator;

    template <class T>
    class list_node
    {
        friend class intrusive_list<T>;
        friend class intrusive_list_iterator<T>;
        T* prev;
        T* next;
    };

    template <class T>
    class intrusive_list_iterator
    {
    public:
        T&operator*();
        T*operator->();
        intrusive_list_iterator operator++(int);
        intrusive_list_iterator&operator++();
        intrusive_list_iterator operator--(int);
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
    intrusive_list_iterator<T> intrusive_list_iterator<T>::operator++(int) {
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
    intrusive_list_iterator<T> intrusive_list_iterator<T>::operator--(int) {
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

        intrusive_list() : m_head(nullptr), m_tail(nullptr) {}

        using iterator_t = intrusive_list_iterator<T>;

        /**
         * Returns the number of elements in the list
         * Traverses the whole list, prefer `empty` if possible
         * @return number of elements
         */
        size_t size() {
            size_t sz {0};
            for (auto it = m_head; it; it = it->next, ++sz);
            return sz;
        }

        /**
         * Returns whether the list is empty or not
         * @return list contains any elements
         */
        bool empty() { return m_head == nullptr; }

        void push_back(T& t);
        void push_front(T& t);

        T& front();
        T& back();

        const T& front() const;
        const T& back() const;

        void insert(iterator_t at, T& t);

        void pop_back();
        void pop_front();

        /**
         * Removes all elements from the container
         */
        void clear();

        iterator_t erase(iterator_t);

        iterator_t begin();
        iterator_t end();
    };

    template<class T>
    bool intrusive_list_iterator<T>::operator!=(const intrusive_list_iterator & rhs) {
        return m_curr != rhs.m_curr;
    }

    template<class T>
    auto intrusive_list<T>::begin() -> iterator_t {
        return iterator_t{m_head};
    }

    template<class T>
    auto intrusive_list<T>::end() -> iterator_t {
        return iterator_t{nullptr};
    }

    template<class T>
    void intrusive_list<T>::push_back(T &t) {
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
    }

    template <class T>
    void intrusive_list<T>::push_front(T& t)
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
    }

    template<class T>
    void intrusive_list<T>::insert(intrusive_list::iterator_t at, T& t)
    {
        if (at == begin())
        {
            push_front(t);
            return;
        }
        else if (at == end())
        {
            push_back(t);
            return;
        }

        t.prev = at->prev;
        at->prev = &t;
        t.next = &(*at);

        if (t.prev)
        {
            t.prev->next = &t;
        }
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

        if (m_head == ptr)
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
}
