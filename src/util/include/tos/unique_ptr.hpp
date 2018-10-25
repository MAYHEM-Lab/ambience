//
// Created by fatih on 10/21/18.
//

#pragma once

#include <tos/utility.hpp>

namespace tos
{
    template <class T>
    struct default_delete
    {
        void operator()(T* ptr) const {
            enum { type_must_be_complete = sizeof *ptr };
            delete ptr;
        }
    };

    template <class T, class DeleterT = default_delete<T>>
    class unique_ptr : DeleterT
    {
    public:
        explicit unique_ptr(T* t) : m_ptr{t} {}
        unique_ptr(T* t, DeleterT&& del) : m_ptr{t}, DeleterT{std::forward(del)} {}

        unique_ptr(const unique_ptr&) = delete;
        unique_ptr(unique_ptr&& rhs) noexcept : m_ptr{rhs.m_ptr} {
            rhs.m_ptr = nullptr;
        }

        T* operator->() { return m_ptr; }
        const T*operator->() const { return m_ptr; }

        T&operator*() { return *m_ptr; }
        const T&operator*() const { return *m_ptr; }

        T* get() { return m_ptr; }
        const T* get() const { return m_ptr; }

        ~unique_ptr()
        {
            (*static_cast<DeleterT*>(this))(m_ptr);
        }

    private:
        T* m_ptr;
    };

    template<typename T, typename... Args>
    tos::unique_ptr<T> make_unique(Args&&... args)
    {
        return tos::unique_ptr<T>(new T(std::forward<Args>(args)...));
    }
}