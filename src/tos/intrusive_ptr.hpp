//
// Created by fatih on 4/11/18.
//

#pragma once

namespace tos
{
    template <class T>
    class intrusive_ptr
    {
    public:
        intrusive_ptr() : m_ptr(nullptr) {}

        intrusive_ptr(decltype(nullptr)) : m_ptr(nullptr) {}

        intrusive_ptr(T* t) : m_ptr(t)
        {
            intrusive_ref(m_ptr);
        }

        intrusive_ptr(const intrusive_ptr& rhs) noexcept : m_ptr(rhs.m_ptr)
        {
            intrusive_ref(m_ptr);
        }

        intrusive_ptr(intrusive_ptr&& rhs) noexcept : m_ptr(rhs.m_ptr)
        {
            rhs.m_ptr = nullptr;
        }

        intrusive_ptr&operator=(const intrusive_ptr& rhs) noexcept
        {
            reset();
            intrusive_ref(m_ptr);
            m_ptr = rhs.m_ptr;

            return *this;
        }

        intrusive_ptr&operator=(intrusive_ptr&& rhs) noexcept
        {
            reset();
            m_ptr = rhs.m_ptr;
            rhs.m_ptr = nullptr;
            return *this;
        }

        T* get() { return m_ptr; }
        const T* get() const { return m_ptr; }

        T*operator->() { return get(); }
        const T*operator->() const { return get(); }

        T&operator*() { return *get(); }
        const T&operator*() const { return *get(); }

        void reset()
        {
            if (!m_ptr) return;
            intrusive_unref(m_ptr);
        }

        ~intrusive_ptr()
        {
            reset();
        }
    private:
        T* m_ptr;
    };
}