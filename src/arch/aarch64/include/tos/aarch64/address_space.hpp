#pragma once

#include <tos/aarch64/mmu.hpp>
#include <tos/address_space.hpp>
#include <tos/memory.hpp>

namespace tos::aarch64 {
struct address_space final : tos::address_space {
    expected<void, mmu_errors> allocate_region(mapping& mapping,
                                               physical_page_allocator* palloc) {
        Assert(mapping.vm_segment.range.base.address() % page_size_bytes == 0);
        Assert(mapping.vm_segment.range.size % page_size_bytes == 0);
        EXPECTED_TRYV(aarch64::allocate_region(
            *m_table, mapping.vm_segment, mapping.allow_user, palloc));
        return {};
    }

    auto do_mapping(mapping& mapping, physical_page_allocator* arg) {
        auto res = allocate_region(mapping, arg);
        if (res) {
            // Only modify internal state if the allocation succeeds.
            add_mapping(mapping);
        }
        return res;
    }

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