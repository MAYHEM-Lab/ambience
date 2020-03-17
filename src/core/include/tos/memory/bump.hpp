#pragma once

#include <tos/span.hpp>
#include <cstdint>
#include <cstddef>
#include <cstring>

namespace tos::memory {
struct bump_allocator {
    explicit bump_allocator(tos::span<uint8_t> buffer)
        : m_buf{buffer}, m_ptr{buffer.begin()} {}

    void* allocate(size_t size) {
        size = size_align(size, 2);

        if (m_ptr + size + sizeof(alloc_header) >= m_buf.end()) {
            return nullptr;
        }
        return push_header(size);
    }

    void* realloc(void* ptr, size_t size) {
        size = size_align(size, 2);
        auto alloc = allocate(size);
        if (alloc && ptr) {
            auto header = get_header(ptr);
            std::memcpy(alloc, ptr, header->size);
        }
        return alloc;
    }

    void deallocate(void*) {
        // deallocate in a bump allocator does nothing
    }

    void reset(uint8_t* to) {
        m_ptr = to;
    }

    uint8_t* get() {
        return m_ptr;
    }

private:
    unsigned size_align(unsigned size, unsigned bit_count_to_zero)
    {
        unsigned bits = (1 << bit_count_to_zero) - 1;
        return (size + bits) & ~bits;
    }

    void* push_header(uint16_t size) {
        auto header = new (m_ptr) alloc_header;
        header->size = size;
        void* res = header + 1;
        m_ptr = reinterpret_cast<uint8_t*>(res) + size;
        return res;
    }

    struct alignas(4) alloc_header {
        uint32_t size;
    };

    alloc_header* get_header(void* allocated) {
        return static_cast<alloc_header*>(allocated) - 1;
    }

    tos::span<uint8_t> m_buf;
    uint8_t* m_ptr;
};

}
