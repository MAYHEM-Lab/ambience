#pragma once

#include <cstddef>
#include <utility>

namespace tos::memory {
struct polymorphic_allocator {
    virtual void* allocate(size_t size) = 0;
    virtual void free(void* ptr) = 0;
    virtual ~polymorphic_allocator() = default;
};

namespace detail {
template<class T>
struct erased_allocator : polymorphic_allocator {
    explicit erased_allocator(T t)
        : m_alloc{std::move(t)} {
    }

    void* allocate(size_t size) override {
        return (&m_alloc)->allocate(size);
    }
    void free(void* ptr) override {
        return (&m_alloc)->free(ptr);
    }

    T m_alloc;
};
} // namespace detail

template<class T>
auto erase_allocator(T&& t) -> detail::erased_allocator<T> {
    return detail::erased_allocator<T>(std::forward<T>(t));
}
} // namespace tos::memory