#pragma once

#include <memory>
#include <tos/span.hpp>

namespace tos::memory {
class buddy_allocator {
public:
    explicit buddy_allocator(tos::span<uint8_t> buffer, uint8_t min_size = 128)
        : m_raw(buffer)
        , m_min_size(min_size) {
    }

    using ptr_type = std::unique_ptr<uint8_t[], void (buddy_allocator::*)(uint8_t*)>;

private:
    void allocate(span<uint8_t> elem, size_t size);

    span<uint8_t> m_raw;
    uint8_t m_min_size;
};
} // namespace tos::memory