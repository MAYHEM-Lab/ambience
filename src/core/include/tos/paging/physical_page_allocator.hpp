#pragma once

#include <tos/expected.hpp>
#include <tos/intrusive_list.hpp>
#include <tos/intrusive_ptr.hpp>
#include <tos/memory.hpp>

namespace tos {
struct mapping;
struct physical_page : ref_counted<physical_page, int8_t, ignore_t> {
    const mapping* map;
    list_node<physical_page> node;

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
    physical_page* allocate(int count, int align = 1);
    void free(span<physical_page> pages);

    // Returns a pointer to the **physical memory** for the given page.
    // You **have to** map this physical memory to an address space before you can access
    // it!
    physical_address address_of(const physical_page& page) const;
    physical_range range_of(const physical_page& page) const;

    physical_range range_of(span<const physical_page> pages) const;

    int page_num(const physical_page& page) const;

    // Marks the given range of physical memory as unavailable.
    void mark_unavailable(const physical_range& len);

    physical_page* info(int32_t page_num);
    physical_page* info(physical_address ptr);

    static constexpr size_t size_for_pages(size_t num_pages) {
        return sizeof(physical_page_allocator) + num_pages * sizeof(physical_page);
    }

    size_t remaining_page_count() const {
        return m_remaining;
    }

    size_t page_size() const {
        return 4096;
    }

    static physical_page_allocator* instance();

private:
    span<physical_page> get_table() {
        return {m_table, m_num_pages};
    }

    span<const physical_page> get_table() const {
        return {m_table, m_num_pages};
    }

    size_t m_remaining;

    size_t m_num_pages;
    intrusive_list<physical_page, through_member<&physical_page::node>> free_list;
    physical_page m_table[];
};
} // namespace tos