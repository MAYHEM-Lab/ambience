#pragma once

#include <tos/address_space.hpp>
#include <tos/arm/fwd.hpp>
#include <tos/arm/mpu.hpp>
#include <tos/mapping.hpp>
#include <tos/memory.hpp>
#include <tos/paging/physical_page_allocator.hpp>

namespace tos::arm {
struct address_space final : tos::address_space {
    expected<void, mpu_errors> allocate_region(mapping& mapping,
                                               physical_page_allocator* palloc) {
        return mpu().set_region(
            1, mapping.vm_segment.range, mapping.vm_segment.perms, true, 0, false);
    }

    expected<void, mpu_errors>
    mark_resident(mapping& mapping, memory_range subrange, physical_address phys_addr) {
        // As there is no virtual memory with an MPU, the physical address must be the
        // same with allocation address (i.e. virtual address), therefore we just ignore
        // it.
        // Subrange support could be implemented in theory, but we don't support it yet.
        Assert(mapping.vm_segment.range == subrange);
        Assert(subrange.base == phys_addr.address());
        mpu().enable_region(1);
        return {};
    }

    friend void activate(address_space& to_activate) {
        tos::global::cur_as = &to_activate;
        mpu().enable();
    }

    memory_range containing_fragment(const memory_range& range) const;
};

template<class... DataPtrTs>
auto create_share(address_space& from,
                  address_space& to,
                  const std::tuple<DataPtrTs...>& in_ptrs);
} // namespace tos::arm