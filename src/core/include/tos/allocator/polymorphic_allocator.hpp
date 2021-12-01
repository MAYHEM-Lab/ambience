#pragma once

#include <cstddef>
#include <utility>
#include <optional>

namespace tos::memory {
struct polymorphic_allocator {
    virtual void* allocate(size_t size) = 0;
    virtual void* realloc(void* oldptr, size_t newsz) = 0;
    virtual void free(void* ptr) = 0;
    virtual std::optional<size_t> in_use() const {
        return {};
    }
    virtual ~polymorphic_allocator() = default;
};

namespace detail {
template<class T>
struct erased_allocator : polymorphic_allocator {
    template <class... Ts>
    explicit erased_allocator(Ts&&... ts)
        : m_alloc{std::forward<Ts>(ts)...} {
    }

    void* allocate(size_t size) override {
        return (&m_alloc)->allocate(size);
    }

    void* realloc(void* oldptr, size_t newsize) override {
        return (&m_alloc)->realloc(oldptr, newsize);
    }
    
    void free(void* ptr) override {
        return (&m_alloc)->free(ptr);
    }

    std::optional<size_t> in_use() const override {
        return (&m_alloc)->in_use();
    }

    T m_alloc;
};
} // namespace detail

template<class T>
auto erase_allocator(T&& t) -> detail::erased_allocator<T> {
    return detail::erased_allocator<T>(std::forward<T>(t));
}
} // namespace tos::memory