#pragma once

#include <tos/aarch64/mmu.hpp>
#include <tos/address_space.hpp>
#include <tos/memory.hpp>

namespace tos::aarch64 {
struct address_space final : tos::address_space {
    expected<void, mmu_errors> allocate_region(mapping& mapping,
                                               physical_page_allocator* palloc);
    expected<void, mmu_errors>
    mark_resident(mapping& mapping, virtual_range subrange, physical_address phys_addr);

    friend void activate(address_space& to_activate) {
        tos::global::cur_as = &to_activate;
        set_current_translation_table(*to_activate.m_table);
    }

    // The cloned address space is owned unlike empty or adopted address spaces as it
    // potentially contains mappings that point back to it.
    expected<std::unique_ptr<address_space>, mmu_errors>
    clone(physical_page_allocator& palloc);

    translation_table* m_table;

    virtual_range containing_fragment(const virtual_range& range) const;
};

template<class... DataPtrTs>
auto create_share(address_space& from,
                  address_space& to,
                  const std::tuple<DataPtrTs...>& in_ptrs);
} // namespace tos::aarch64