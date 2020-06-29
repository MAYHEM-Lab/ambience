#pragma once
#include <lidl/traits.hpp>

namespace lidl {
template<class T, bool IsTReference = is_ptr<T>{} || is_reference_type<T>{}>
class vector;

template<class T>
struct is_reference_type<vector<T>> : std::true_type {};

template<class T>
class vector<T, false> {
public:
    explicit vector(int16_t len)
        : m_len{len} {
        //        std::uninitialized_default_construct_n(begin(), size());
    }

    [[nodiscard]] tos::span<T> span() {
        return tos::span<T>(data(), size());
    }

    [[nodiscard]] tos::span<const T> span() const {
        return tos::span<const T>(data(), size());
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
    [[nodiscard]] size_t size() const {
        return m_len;
        //        return m_ptr.get_offset() / sizeof(T);
    }

    T* data()  {
        auto potential_begin = reinterpret_cast<char*>(
            (&m_len) + 1); // the string begins after the length.
        // Align the begin pointer for the type.
        while (reinterpret_cast<uintptr_t>(potential_begin) % alignof(T) != 0) {
            ++potential_begin;
        }
        return reinterpret_cast<T*>(potential_begin);
    }

    const T* data() const {
        auto potential_begin = reinterpret_cast<const char*>(
            (&m_len) + 1); // the string begins after the length.
        // Align the begin pointer for the type.
        while (reinterpret_cast<uintptr_t>(potential_begin) % alignof(T) != 0) {
            ++potential_begin;
        }
        return reinterpret_cast<const T*>(potential_begin);
    }

    int16_t m_len;
};

class vector_iterator_end {};

template<class T>
class vector_iterator {
public:
    vector_iterator(ptr<T>* cur, ptr<T>* end)
        : m_cur{cur}
        , m_end{end} {
    }

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
    friend bool operator==(const vector_iterator<T>& it, vector_iterator_end) {
        return it.m_cur == it.m_end;
    }

    friend bool operator!=(const vector_iterator<T>& it, vector_iterator_end) {
        return it.m_cur != it.m_end;
    }

    ptr<T>* m_cur;
    ptr<T>* m_end;
};

template<class T>
class vector<ptr<T>, true> {
public:
    explicit vector(int16_t base)
        : m_under{base} {
    }

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
    [[nodiscard]] size_t size() const {
        return m_under.size();
    }

    vector<ptr<T>, false> m_under;
};

template<class T>
class vector<T, true> : vector<ptr<T>, true> {
    using vector<ptr<T>, true>::vector;
};

template<class T, std::enable_if_t<is_ptr<T>{}>* = nullptr>
vector<T>& create_vector_sized(message_builder& builder, int size) {
    auto& vec = emplace_raw<vector<T>>(builder, int16_t(size));
    builder.allocate(size * sizeof(T), alignof(T));
    return vec;
}

template<class T, std::enable_if_t<!is_ptr<T>{} && !is_reference_type<T>{}>* = nullptr>
vector<T>& create_vector_sized(message_builder& builder, int size) {
    auto& vec = emplace_raw<vector<T>>(builder, int16_t(size));
    builder.allocate(size * sizeof(T), alignof(T));
    return vec;
}

template<class T, std::enable_if_t<!is_ptr<T>{} && !is_reference_type<T>{}>* = nullptr>
vector<T>& create_vector(message_builder& builder, tos::span<const T> elems) {
    auto& vec = create_vector_sized<T>(builder, elems.size());
    copy(vec.span(), elems);
    return vec;
}

template<class T, std::enable_if_t<is_ptr<T>{}>* = nullptr>
vector<T>& create_vector(message_builder& builder, typename T::element_type& elem) {
    auto& vec = create_vector_sized<T>(builder, 1);
    vec.get_raw().span()[0] = elem;
    return vec;
}

template<class T, std::enable_if_t<is_ptr<T>{}>* = nullptr>
vector<T>& create_vector(message_builder& builder,
                         typename T::element_type& elem,
                         typename T::element_type& elem1) {
    auto& vec = create_vector_sized<T>(builder, 2);
    vec.get_raw().span()[0] = elem;
    vec.get_raw().span()[1] = elem1;
    return vec;
}

template<class T, std::enable_if_t<is_reference_type<T>{}>* = nullptr>
vector<ptr<T>>& create_vector(message_builder& builder, T& elem) {
    return create_vector<ptr<T>>(builder, elem);
}

template<class T, std::enable_if_t<is_reference_type<T>{}>* = nullptr>
vector<ptr<T>>& create_vector(message_builder& builder, T& elem, T& elem2) {
return create_vector<ptr<T>>(builder, elem, elem2);
}
} // namespace lidl
