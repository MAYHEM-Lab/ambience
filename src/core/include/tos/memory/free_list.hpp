#pragma once

#include <cstddef>
#include <cstdint>
#include <tos/intrusive_list.hpp>
#include <tos/span.hpp>

namespace tos::memory {
struct free_header;
class free_list {
public:
    explicit free_list(span<uint8_t> buffer);

    void* allocate(size_t size);
    void free(void* ptr);

    size_t available_memory() const {
        return m_buffer.size() - m_used;
    }

private:
    void add_block(free_header&);
    span<uint8_t> m_buffer;
    intrusive_list<free_header> m_list;
    size_t m_used = 0;
};
} // namespace tos::memory