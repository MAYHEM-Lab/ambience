#pragma once

#include <tos/mapping.hpp>
#include <tos/memory.hpp>
#include <tos/x86_64/exception.hpp>
#include <tos/x86_64/mmu.hpp>

namespace tos::x86_64 {
struct address_space {
    expected<void, mmu_errors> allocate_region(mapping& mapping) {
        mapping.backend_data = this;
        Assert(mapping.vm_segment.range.base % 4096 == 0);
        Assert(mapping.vm_segment.range.size % 4096 == 0);
        EXPECTED_TRYV(x86_64::allocate_region(
            *m_table, mapping.vm_segment, mapping.allow_user, palloc));
        return {};
    }

    expected<void, mmu_errors>
    mark_resident(mapping& mapping, memory_range subrange, void* phys_addr) {
        return x86_64::mark_resident(*m_table, subrange, mapping.mem_type, phys_addr);
    }

    expected<bool, mmu_errors> handle_memory_fault(const exception_frame& frame,
                                                   uintptr_t fault_addr);

    static memory_range containing_fragment(memory_range range) {
        range.size = align_nearest_up_pow2(range.size, page_size_bytes);
        range.base = align_nearest_down_pow2(range.base, page_size_bytes);
        return range;
    }

    tos::address_space* space;
    physical_page_allocator* palloc;
    translation_table* m_table;
};
} // namespace tos::x86_64