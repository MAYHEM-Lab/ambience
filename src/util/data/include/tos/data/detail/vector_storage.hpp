#pragma once

#include <tos/span.hpp>
#include <vector>

namespace tos::data {
template<class Type>
struct vector_storage {
    explicit vector_storage(int size)
        : m_elements(size) {
    }

    std::vector<Type> m_elements;

    constexpr span<Type> elements() {
        return m_elements;
    }
};
} // namespace tos::data