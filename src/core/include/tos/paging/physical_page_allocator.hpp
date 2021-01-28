#pragma once

#include <tos/memory.hpp>

namespace tos {
struct physical_page : ref_counted<physical_page> {
    const mapping* map;

    bool free() const {
        return reference_count() == 0;
    }
};

class physical_page_allocator {
public:
    explicit physical_page_allocator(size_t num_pages);

    // Allocates the given number of physical pages on the specified alignment.
    // Both arguments are in numbers of physical pages.
    // The alignment argument must be a power of 2.
    intrusive_ptr<physical_page> allocate(int count, int align = 1);

    void* address_of(const physical_page& page) const;

    int page_num(const physical_page& page) const;

    // Marks the given range of physical memory as unavailable.
    void mark_unavailable(const memory_range& len);

    physical_page* info(int32_t page_num);
    physical_page* info(void* ptr);


    static constexpr size_t size_for_pages(size_t num_pages) {
        return sizeof(physical_page_allocator) + num_pages * sizeof(physical_page);
    }

    size_t remaining_page_count() const {
        return m_remaining;
    }

private:
    span<physical_page> get_table() {
        return {m_table, m_num_pages};
    }

    span<const physical_page> get_table() const {
        return {m_table, m_num_pages};
    }

    size_t m_remaining;

    size_t m_num_pages;
    physical_page m_table[];
};
}