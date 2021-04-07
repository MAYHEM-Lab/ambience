//
// Created by fatih on 4/11/18.
//

#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <type_traits>
#include <utility>

namespace tos {
template<class T>
class intrusive_ptr {
    template<class U>
    friend class intrusive_ptr;

public:
    using element_type = T;

    /**
     * Constructs a null pointer.
     */
    constexpr intrusive_ptr()
        : m_ptr(nullptr) {
    }

    constexpr intrusive_ptr(std::nullptr_t)
        : m_ptr(nullptr) {
    }

    template<class U, std::enable_if_t<std::is_convertible_v<U*, T*>>* = nullptr>
    constexpr explicit intrusive_ptr(U* t)
        : m_ptr(t) {
        intrusive_ref(m_ptr);
    }

    template<class U, std::enable_if_t<std::is_convertible_v<U*, T*>>* = nullptr>
    constexpr intrusive_ptr(const intrusive_ptr<U>& rhs)
        : m_ptr(rhs.m_ptr) {
        intrusive_ref(m_ptr);
    }

    template<class U, std::enable_if_t<std::is_convertible_v<U*, T*>>* = nullptr>
    constexpr intrusive_ptr(intrusive_ptr<U>&& rhs)
        : m_ptr(rhs.m_ptr) {
        rhs.m_ptr = nullptr;
    }

    constexpr intrusive_ptr(const intrusive_ptr& rhs) noexcept
        : m_ptr(rhs.m_ptr) {
        intrusive_ref(m_ptr);
    }

    constexpr intrusive_ptr(intrusive_ptr&& rhs) noexcept
        : m_ptr(rhs.m_ptr) {
        rhs.m_ptr = nullptr;
    }

    constexpr intrusive_ptr& operator=(const intrusive_ptr& rhs) noexcept {
        if (&rhs == this) {
            return *this;
        }

        reset();
        intrusive_ref(m_ptr);
        m_ptr = rhs.m_ptr;

        return *this;
    }

    constexpr intrusive_ptr& operator=(intrusive_ptr&& rhs) noexcept {
        if (&rhs == this) {
            return *this;
        }

        reset();
        m_ptr = rhs.m_ptr;
        rhs.m_ptr = nullptr;
        return *this;
    }

    constexpr T* get() {
        return m_ptr;
    }

    constexpr T* get() const {
        return m_ptr;
    }

    constexpr T* operator->() {
        return get();
    }

    constexpr T* operator->() const {
        return get();
    }

    constexpr  T& operator*() {
        return *get();
    }

    constexpr  T& operator*() const {
        return *get();
    }

    constexpr explicit operator bool() {
        return m_ptr;
    }

    constexpr void reset() {
        if (!m_ptr)
            return;
        intrusive_unref(m_ptr);
    }

    constexpr ~intrusive_ptr() {
        reset();
    }

private:
    T* m_ptr;
};

template<class T>
constexpr bool operator==(const intrusive_ptr<T>& left, const intrusive_ptr<T>& right) {
    return left.get() == right.get();
}

template<class T>
constexpr bool operator!=(const intrusive_ptr<T>& left, const intrusive_ptr<T>& right) {
    return left.get() != right.get();
}

template<class T>
constexpr bool operator==(const intrusive_ptr<T>& left, std::nullptr_t) {
    return left.get() == nullptr;
}

template<class T>
constexpr bool operator!=(std::nullptr_t, const intrusive_ptr<T>& right) {
    return nullptr != right.get();
}

template<class U, class T>
constexpr intrusive_ptr<U> static_pointer_cast(intrusive_ptr<T> ptr) {
    static_assert(std::is_base_of_v<T, U>);
    return intrusive_ptr<U>(static_cast<U*>(ptr.get()));
}

template<class T, class... ArgTs>
intrusive_ptr<T> make_intrusive(ArgTs&&... args) {
    return intrusive_ptr<T>(new T(std::forward<ArgTs>(args)...));
}

template<class T, class RefCntT = int8_t, class Deleter = std::default_delete<T>>
class ref_counted {
public:
    constexpr friend void intrusive_ref(T* t) {
        static_cast<ref_counted*>(t)->m_refcnt++;
    }

    constexpr friend void intrusive_unref(T* t) {

        static_cast<ref_counted*>(t)->m_refcnt--;
        if (static_cast<ref_counted*>(t)->m_refcnt == 0) {
            T::collect(t);
        }
    }

    constexpr static void collect(T* channel) {
        Deleter{}(channel);
    }

    constexpr RefCntT reference_count() const {
        return m_refcnt;
    }

private:
    RefCntT m_refcnt{0};
};
} // namespace tos