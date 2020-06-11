#pragma once
#include <lidl/traits.hpp>

namespace lidl {
template <class T, bool IsTReference = is_ptr<T>{} || is_reference_type<T>{}>
class vector;

template <class T> struct is_reference_type<vector<T>> : std::true_type {};

template <class T>
class vector<T, false> {
public:
    explicit vector(T* base) : m_ptr{base} {
        std::uninitialized_default_construct_n(begin(), size());
    }

    [[nodiscard]]
    tos::span<T> span()  {
        auto base = &m_ptr.unsafe().get();
        return tos::span<T>(base, size());
    }

    [[nodiscard]]
    tos::span<const T> span() const {
        auto base = &m_ptr.unsafe().get();
        return tos::span<const T>(base, size());
    }

    operator tos::span<T>() {
        return span();
    }

    operator tos::span<const T>() const {
        return span();
    }

    auto begin() {
        return span().begin();
    }

    auto end() {
        return span().end();
    }

    auto begin() const {
        return span().begin();
    }

    auto end() const {
        return span().end();
    }

private:
    [[nodiscard]]
    size_t size() const {
        return m_ptr.get_offset() / sizeof(T);
    }

    ptr<T> m_ptr{nullptr};
};

class vector_iterator_end {
};

template <class T>
class vector_iterator {
public:
    vector_iterator(ptr<T>* cur, ptr<T>* end) : m_cur{cur}, m_end{end} {}

    T& operator*() {
        return m_cur->unsafe().get();
    }

    vector_iterator& operator++() {
        ++m_cur;
        return *this;
    }

    vector_iterator operator++(int) {
        auto copy = *this;
        ++(*this);
        return copy;
    }

private:
    friend
    bool operator==(const vector_iterator<T>& it, vector_iterator_end) {
        return it.m_cur == it.m_end;
    }

    friend
    bool operator!=(const vector_iterator<T>& it, vector_iterator_end) {
        return it.m_cur != it.m_end;
    }

    ptr<T>* m_cur;
    ptr<T>* m_end;
};

template <class T>
class vector<ptr<T>, true> {
public:
    explicit vector(ptr<T>* base) : m_under{base} {}

    T& operator[](int i) {
        return m_under.span()[i].unsafe();
    }

    vector_iterator<T> begin() {
        return {m_under.begin(), m_under.end()};
    }

    vector_iterator<T> begin() const {
        return {m_under.begin(), m_under.end()};
    }

    vector_iterator_end end() const {
        return {};
    }

    auto& get_raw() {
        return m_under;
    }

private:
    [[nodiscard]]
    size_t size() const {
        return m_under.size();
    }

    vector<ptr<T>, false> m_under;
};

template <class T>
class vector<T, true> : vector<ptr<T>, true> {
    using vector<ptr<T>, true>::vector;
};

template <class T, std::enable_if_t<!is_ptr<T>{} && !is_reference_type<T>{}>* = nullptr>
vector<T>& create_vector_sized(message_builder& builder, int size) {
    auto padding = (builder.size() + size * sizeof(T)) % alignof(vector<T>);
    builder.allocate(padding, 1);
    auto alloc = builder.allocate(size * sizeof(T), alignof(T));
    auto& res = emplace_raw<vector<T>>(builder, reinterpret_cast<T*>(alloc));
    return res;
}

template <class T, std::enable_if_t<!is_ptr<T>{} && !is_reference_type<T>{}>* = nullptr>
vector<T>& create_vector(message_builder& builder, tos::span<const T> elems) {
    auto padding = (builder.size() + elems.size() * sizeof(T)) % alignof(vector<T>);
    builder.allocate(padding, 1);
    auto alloc = builder.allocate(elems.size() * sizeof(T), alignof(T));
    auto& res = emplace_raw<vector<T>>(builder, reinterpret_cast<T*>(alloc));
    copy(res.span(), elems);
    return res;
}

template <class T, std::enable_if_t<is_ptr<T>{}>* = nullptr>
vector<T>& create_vector(message_builder& builder, typename T::element_type& elem) {
    auto alloc = builder.allocate(1 * sizeof(T), alignof(T));
    auto& res = emplace_raw<vector<T>>(builder, reinterpret_cast<T*>(alloc));
    res.get_raw().span()[0] = elem;
    return res;
}

template <class T, std::enable_if_t<is_ptr<T>{}>* = nullptr>
vector<T>& create_vector(message_builder& builder, typename T::element_type& elem,typename T::element_type& elem1) {
    auto alloc = builder.allocate(2 * sizeof(T), alignof(T));
    auto& res = emplace_raw<vector<T>>(builder, reinterpret_cast<T*>(alloc));
    res.get_raw().span()[0] = elem;
    res.get_raw().span()[1] = elem1;
    return res;
}

template <class T, std::enable_if_t<is_reference_type<T>{}>* = nullptr>
vector<ptr<T>>& create_vector(message_builder& builder, T& elem) {
    return create_vector<ptr<T>>(builder, elem);
}
}
