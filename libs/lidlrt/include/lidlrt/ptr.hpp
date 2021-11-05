#pragma once

#include <cstdint>
#include <type_traits>
#include <iterator>

namespace lidl {
template<class T>
class alignas(2) ptr {
    struct unsafe_;

public:
    using element_type = T;

    explicit ptr()
        : m_unsafe{0} {
    }

    explicit ptr(std::nullptr_t)
        : ptr() {
    }

    explicit ptr(const uint8_t* to)
        : ptr(reinterpret_cast<uint8_t*>(this) - to) {
    }

    template<class U = T, std::enable_if_t<!std::is_same_v<U, uint8_t*>>* = nullptr>
    explicit ptr(const T& to)
        : m_unsafe{int16_t(reinterpret_cast<const uint8_t*>(this) -
                           reinterpret_cast<const uint8_t*>(&to))} {
    }

    template<class U = T, std::enable_if_t<!std::is_same_v<U, uint8_t>>* = nullptr>
    explicit ptr(const T* to)
        : ptr(reinterpret_cast<const uint8_t*>(this) -
              reinterpret_cast<const uint8_t*>(to)) {
    }

    explicit ptr(int16_t offset)
        : m_unsafe{offset} {
    }

    ptr(const ptr&) = delete;
    ptr(ptr&&) = delete;

    [[nodiscard]] int16_t get_offset() const {
        return m_unsafe.m_offset;
    }

    [[nodiscard]] unsafe_& unsafe() {
        return m_unsafe;
    }

    [[nodiscard]] const unsafe_& unsafe() const {
        return m_unsafe;
    }

    [[nodiscard]] explicit operator bool() const {
        return get_offset() != 0;
    }

    ptr& operator=(const T& to) {
        m_unsafe.m_offset = reinterpret_cast<const uint8_t*>(this) -
                            reinterpret_cast<const uint8_t*>(&to);
        return *this;
    }

    operator T&() {
        return unsafe().get();
    }

    operator const T&() const {
        return unsafe().get();
    }

private:
    struct unsafe_ {
        // Stores the offset in number of bytes, not Ts!
        int16_t m_offset;

        const T& get() const {
            auto self = reinterpret_cast<const uint8_t*>(this);
            auto ptr = self - m_offset;
            return *reinterpret_cast<const T*>(ptr);
        }

        T& get() {
            auto self = reinterpret_cast<uint8_t*>(this);
            auto ptr = self - m_offset;
            return *reinterpret_cast<T*>(ptr);
        }

        T* operator->() {
            return &get();
        }
        const T* operator->() const {
            return &get();
        }
    } m_unsafe;
};

template<class T>
class const_ptr_iterator {
public:
    using value_type = T;
    using iterator_category = std::forward_iterator_tag;
    using difference_type = T;
    using pointer = T*;
    using reference = T&;

    const_ptr_iterator(const ptr<T>* cur)
        : m_cur{cur} {
    }

    const T& operator*() const {
        return m_cur->unsafe().get();
    }

    const_ptr_iterator& operator++() {
        ++m_cur;
        return *this;
    }

    const_ptr_iterator operator++(int) {
        auto copy = *this;
        ++(*this);
        return copy;
    }

private:
    friend bool operator==(const const_ptr_iterator<T>& it,
                           const const_ptr_iterator<T>& other_it) {
        return it.m_cur == other_it.m_cur;
    }

    friend bool operator!=(const const_ptr_iterator<T>& it,
                           const const_ptr_iterator<T>& other_it) {
        return it.m_cur != other_it.m_cur;
    }

    const ptr<T>* m_cur;
};

template<class T>
class ptr_iterator {
public:
    using value_type = T;
    using iterator_category = std::forward_iterator_tag;
    using difference_type = T;
    using pointer = T*;
    using reference = T&;

    ptr_iterator(ptr<T>* cur)
        : m_cur{cur} {
    }

    T& operator*() {
        return m_cur->unsafe().get();
    }

    ptr_iterator& operator++() {
        ++m_cur;
        return *this;
    }

    ptr_iterator operator++(int) {
        auto copy = *this;
        ++(*this);
        return copy;
    }

private:
    friend bool operator==(const ptr_iterator<T>& it,
                           const ptr_iterator<T>& other_it) {
        return it.m_cur == other_it.m_cur;
    }

    friend bool operator!=(const ptr_iterator<T>& it,
                           const ptr_iterator<T>& other_it) {
        return it.m_cur != other_it.m_cur;
    }

    ptr<T>* m_cur;
};

static_assert(sizeof(ptr<int>) == 2);
static_assert(alignof(ptr<int>) == 2);
} // namespace lidl