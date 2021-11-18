#pragma once

#include "tos/debug/trace/counter.hpp"
#include "tos/utility.hpp"
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
        return m_used_ctr.get();
    }
    std::optional<size_t> peak_use() const {
        return peak_used_memory();
    }
    std::optional<size_t> capacity() const {
        return m_buffer.size();
    }


    [[nodiscard]]
    size_t available_memory() const {
        return m_buffer.size() - m_used_ctr.get();
    }

    [[nodiscard]]
    size_t peak_used_memory() const {
        return m_peak_memory.get();
    }

private:
    void add_block(free_header&);
    span<uint8_t> m_buffer;
    intrusive_list<free_header> m_list;
    trace::gauge m_used_ctr{"free_list_in_use_memory"};
    trace::counter m_peak_memory{"free_list_peak_memory"};
};
} // namespace tos::memory