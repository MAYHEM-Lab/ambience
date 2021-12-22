#pragma once

#include <cstddef>
#include <cstdint>
#include <tos/intrusive_list.hpp>
#include <tos/span.hpp>
#include <optional>

namespace tos::memory {
struct free_header;

class free_list {
public:
    explicit free_list(span<uint8_t> buffer);

    void* allocate(size_t size);
    void* realloc(void* oldptr, size_t size);
    void free(void* ptr);
    std::optional<size_t> in_use() const {
        return m_used;
    }
    std::optional<size_t> peak_use() const {
        return m_peak;
    }
    std::optional<size_t> capacity() const {
        return m_buffer.size();
    }


    [[nodiscard]]
    size_t available_memory() const {
        return m_buffer.size() - m_used;
    }

    [[nodiscard]]
    size_t peak_used_memory() const {
        return m_peak;
    }

private:
    void add_block(free_header&);
    span<uint8_t> m_buffer;
    intrusive_list<free_header> m_list;
    size_t m_used = 0;
    size_t m_peak = 0;
};
} // namespace tos::memory