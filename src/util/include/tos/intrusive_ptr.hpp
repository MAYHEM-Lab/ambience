//
// Created by fatih on 4/11/18.
//

#pragma once

#include <cstddef>
#include <type_traits>

namespace tos {
template<class T>
class intrusive_ptr {
public:
    using element_type = T;

    /**
     * Constructs a null pointer.
     */
    intrusive_ptr()
        : m_ptr(nullptr) {
    }

    intrusive_ptr(std::nullptr_t)
        : m_ptr(nullptr) {
    }

    template <class U, std::enable_if_t<std::is_convertible_v<U*, T*>>* = nullptr>
    explicit intrusive_ptr(U* t)
        : m_ptr(t) {
        intrusive_ref(m_ptr);
    }

    intrusive_ptr(const intrusive_ptr& rhs) noexcept
        : m_ptr(rhs.m_ptr) {
        intrusive_ref(m_ptr);
    }

    intrusive_ptr(intrusive_ptr&& rhs) noexcept
        : m_ptr(rhs.m_ptr) {
        rhs.m_ptr = nullptr;
    }

    intrusive_ptr& operator=(const intrusive_ptr& rhs) noexcept {
        if (&rhs == this) {
            return *this;
        }

        reset();
        intrusive_ref(m_ptr);
        m_ptr = rhs.m_ptr;

        return *this;
    }

    intrusive_ptr& operator=(intrusive_ptr&& rhs) noexcept {
        if (&rhs == this) {
            return *this;
        }

        reset();
        m_ptr = rhs.m_ptr;
        rhs.m_ptr = nullptr;
        return *this;
    }

    T* get() {
        return m_ptr;
    }

    const T* get() const {
        return m_ptr;
    }

    T* operator->() {
        return get();
    }

    const T* operator->() const {
        return get();
    }

    T& operator*() {
        return *get();
    }

    const T& operator*() const {
        return *get();
    }

    explicit operator bool() {
        return m_ptr;
    }

    void reset() {
        if (!m_ptr)
            return;
        intrusive_unref(m_ptr);
    }

    ~intrusive_ptr() {
        reset();
    }

private:
    T* m_ptr;
};

template <class U, class T>
intrusive_ptr<U> static_pointer_cast(const intrusive_ptr<T>& ptr) {
    static_assert(std::is_convertible_v<T*, U*>);
    return intrusive_ptr<U>(static_cast<U*>(ptr.get()));
}

template <class T, class... ArgTs>
intrusive_ptr<T> make_intrusive(ArgTs&&... args) {
    return intrusive_ptr(new T(std::forward<ArgTs>(args)...));
}
} // namespace tos