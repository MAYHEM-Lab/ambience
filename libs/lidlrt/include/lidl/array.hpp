#pragma once

#include <array>
#include <cstdint>
#include <lidl/ptr.hpp>
#include <lidl/traits.hpp>

namespace lidl {
template<class T,
         std::size_t N,
         bool IsTReference = is_ptr<T>{} || is_reference_type<T>{}>
class array;

template<class T, std::size_t N>
class array<T, N, false> : public std::array<T, N> {};

template<class T, std::size_t N>
class array<ptr<T>, N, true> {
public:
    T& operator[](int i) {
        return m_private[i].unsafe();
    }

    ptr_iterator<T> begin() {
        return ptr_iterator<T>(m_private.data());
    }

    ptr_iterator<T> begin() const {
        return ptr_iterator<T>(m_private.data());
    }

    ptr_iterator<T> end() {
        return ptr_iterator<T>(m_private.data() + m_private.size());
    }

    ptr_iterator<T> end() const {
        return ptr_iterator<T>(m_private.data() + m_private.size());
    }

    auto& get_raw() {
        return m_private;
    }

    [[nodiscard]] std::size_t size() const {
        return m_private.size();
    }

private:
    std::array<ptr<T>, N> m_private;
};

template<class T, std::size_t N>
inline bool operator==(const array<T, N>& left, const array<T, N>& right) {
    return std::equal(left.begin(), left.end(), right.begin());
}
} // namespace lidl