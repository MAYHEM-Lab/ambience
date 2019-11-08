//
// Created by fatih on 5/24/18.
//

#pragma once

#include <array>
#include <cstddef>
#include <stddef.h>
#include <vector>

namespace tos {
/**
 * Span is a class template that represents a view to a
 * contiguous range of values.
 *
 * Span must be used in place of pointer-size pairs in
 * APIs.
 *
 * As this is a view, spans do not own the memory they
 * are pointing to. Care must be taken not to have
 * dangling spans.
 *
 * @tparam T type of the values in the span
 */
template<class T>
class span {
public:
    using iterator = T*;

    /**
     * Constructs an empty span
     */
    explicit constexpr span(std::nullptr_t)
        : span(static_cast<T*>(nullptr), size_t(0)){};

    /**
     * Constructs a span from the given pointer-length pair.
     *
     * Use of this constructor should be avoided in favor of
     * direct conversions from containers.
     *
     * @param base pointer to the beginning of the range
     * @param len size of the range
     */
    constexpr span(T* base, size_t len)
        : m_base(base)
        , m_len(len) {
    }

    /**
     * Constructs a span from the given iterator pair.
     *
     * As usual, the iterators are interpreted as [begin, end),
     * ie. *end is not included.
     *
     * @param base begin iterator
     * @param end end iterator
     */
    constexpr span(T* base, T* end)
        : m_base{base}
        , m_len{end - base} {
    }

    /**
     * Constructs a span from the given C-style array object
     *
     * When passed string literals, this constructor is used,
     * thus, it will have a zero terminator at the end.
     *
     * @tparam Sz size of the array
     * @param arr the array
     */
    template<size_t Sz>
    constexpr span(T (&arr)[Sz])
        : m_base(arr)
        , m_len(Sz) {
    }

    /**
     * Constructs a span from the given C++ array object
     *
     * @tparam Sz size of the array
     * @param arr the array
     */
    template<size_t Sz>
    constexpr span(std::array<T, Sz>& arr)
        : m_base(arr.data())
        , m_len(arr.size()) {
    }

    /**
     * Constructs a const span from the given C++ array to const objects
     *
     * @tparam Sz size of the array
     * @param arr the array
     */
    template<size_t Sz>
    constexpr span(const std::array<std::remove_const_t<T>, Sz>& arr)
        : m_base(arr.data())
        , m_len(arr.size()) {
    }

    /**
     * Constructs a span from the given C++ vector object
     *
     * @param arr the vector
     */
    constexpr span(std::vector<T>& arr)
        : m_base(arr.data())
        , m_len(arr.size()) {
    }
    /**
     * Constructs a const span from the given C++ vector to const objects
     *
     * @param arr the vector
     */
    constexpr span(const std::vector<std::remove_const_t<T>>& arr)
        : m_base(arr.data())
        , m_len(arr.size()) {
    }

    /**
     * Returns the number of elements in the span
     * @return tne number of elements
     */
    constexpr size_t size() const {
        return m_len;
    }

    /**
     * Returns the total number bytes occupied by the elements in the spen
     * @return number of bytes
     */
    constexpr size_t size_bytes() const {
        return size() * sizeof(T);
    }

    /**
     * Returns the pointer to the beginning of the range of the span
     * @return pointer to the beginning of the range
     */
    constexpr T* data() {
        return m_base;
    }

    /**
     * Returns the pointer to the beginning of the range of the span
     * @return pointer to the beginning of the range
     */
    constexpr const T* data() const {
        return m_base;
    }

    constexpr T& operator[](size_t ind) {
        return m_base[ind];
    }

    constexpr const T& operator[](size_t ind) const {
        return m_base[ind];
    }

    constexpr T* begin() {
        return m_base;
    }
    constexpr T* end() {
        return m_base + m_len;
    }

    constexpr const T* begin() const {
        return m_base;
    }
    constexpr const T* end() const {
        return m_base + m_len;
    }

    constexpr bool empty() const {
        return m_len == 0;
    }

    /*std::enable_if_t<!std::is_const_v<T>, span<uint8_t>> as_bytes() {
        return span<uint8_t>{reinterpret_cast<uint8_t*>(data()), size() * sizeof(T)};
    }*/

    span<const uint8_t> as_bytes() const {
        return span<const uint8_t>{reinterpret_cast<const uint8_t*>(data()),
                                   size() * sizeof(T)};
    }

    T& front() {
        return *begin();
    }
    const T& front() const {
        return *begin();
    }

    /**
     * Spans convert to a const version of them automatically
     *
     * @return constant version of this span
     */
    constexpr operator span<const T>() const {
        return {m_base, size_t(m_len)};
    }

    /**
     * Takes a slice from this span.
     *
     * @param begin beginning index of the slice
     * @param len length of the slice
     * @return a new span
     */
    constexpr span slice(size_t begin, size_t len) {
        return {m_base + begin, len};
    }

    /**
     * Takes a slice from this span.
     *
     * @param begin beginning index of the slice
     * @return a new span
     */
    constexpr span slice(size_t begin) {
        return {m_base + begin, size() - begin};
    }

private:
    T* m_base;
    ptrdiff_t m_len;
};

template<class T>
span<T> empty_span() {
    return span<T>(nullptr);
}

/**
 * Constructs a span of size 1 from a single object
 * @tparam T type of the object
 * @param t object
 * @return a span containing t
 */
template<class T>
span<T> monospan(T& t) {
    return span<T>(&t, 1);
}

template<class U, class T>
span<U> spanify(T&& t) {
    return span<U>(std::forward<T>(t));
}

template<class T, class U>
span<T> raw_cast(span<U> sp) {
    static_assert(sizeof(T) == 1, "");
    return {reinterpret_cast<T*>(sp.data()), sp.size() * sizeof(U)};
}

template<class T>
constexpr bool operator==(tos::span<const T> left, span<const T> right) {
    if (left.size() != right.size()) {
        return false;
    }

    return std::equal(left.begin(), left.end(), right.begin());
}

template<class T>
constexpr bool operator==(tos::span<T> left, span<const T> right) {
    return static_cast<tos::span<const T>>(left) == right;
}

template<class T>
constexpr bool operator==(tos::span<const T> left, span<T> right) {
    return left == static_cast<tos::span<const T>>(right);
}

template<class T>
constexpr bool operator==(tos::span<T> left, span<T> right) {
    return static_cast<tos::span<const T>>(left) ==
           static_cast<tos::span<const T>>(right);
}
} // namespace tos