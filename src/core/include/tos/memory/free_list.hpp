#pragma once

#include <tos/span.hpp>
#include <tos/intrusive_list.hpp>

namespace tos::memory {
struct free_header;
class free_list {
public:
    explicit free_list(span<uint8_t> buffer);

    void* allocate(size_t size);
    void free(void* ptr);

private:
    span<uint8_t> m_buffer;
    intrusive_list<free_header> m_list;
};
}