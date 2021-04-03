#pragma once

#include <tos/arm/mpu.hpp>
#include <tos/mapping.hpp>
#include <tos/memory.hpp>
#include <tos/paging/physical_page_allocator.hpp>

namespace tos::arm {
struct address_space {
    expected<void, mpu_errors> allocate_region(mapping& mapping,
                                               physical_page_allocator* palloc) {
        return mpu().set_region(
            1, mapping.vm_segment.range, mapping.vm_segment.perms, true, 0, false);
    }

    expected<void, mpu_errors>
    mark_resident(mapping& mapping, memory_range subrange, void* phys_addr) {
        mpu().enable_region(1);
        return {};
    }

    memory_range containing_fragment(const memory_range& range) const;

    tos::address_space* space;
};
} // namespace tos::arm