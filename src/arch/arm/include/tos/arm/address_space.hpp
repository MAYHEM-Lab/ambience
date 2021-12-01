#pragma once

#include <tos/address_space.hpp>
#include <tos/arm/fwd.hpp>
#include <tos/arm/mpu.hpp>
#include <tos/mapping.hpp>
#include <tos/memory.hpp>
#include <tos/paging/physical_page_allocator.hpp>
#include <tos/quik.hpp>

namespace tos::arm {
struct address_space final : tos::address_space {
    expected<void, mpu_errors> allocate_region(mapping& mapping,
                                               physical_page_allocator* palloc) {
        return mpu().set_region(
            1, mapping.vm_segment.range, mapping.vm_segment.perms, true, 0, false);
    }

    expected<void, mpu_errors>
    mark_resident(mapping& mapping, virtual_range subrange, physical_address phys_addr) {
        // As there is no virtual memory with an MPU, the physical address must be the
        // same with allocation address (i.e. virtual address), therefore we just ignore
        // it.
        // Subrange support could be implemented in theory, but we don't support it yet.
        Assert(mapping.vm_segment.range == subrange);
        Assert(subrange.base == identity_map(phys_addr));
        mpu().enable_region(1);
        return {};
    }

    friend void activate(address_space& to_activate) {
        tos::global::cur_as = &to_activate;
        mpu().enable();
    }

    virtual_range containing_fragment(const virtual_range& range) const;
};

struct common_share : quik::share_base {};

template<class... Ts>
struct typed_share : common_share {
    std::tuple<Ts...>* ptrs;
    void* get_tuple_ptr() override {
        return ptrs;
    }
};

template<class... DataPtrTs>
typed_share<DataPtrTs...> create_share(address_space& from,
                                       address_space& to,
                                       const std::tuple<DataPtrTs...>& in_ptrs) {
    typed_share<DataPtrTs...> share;
    share.ptrs = const_cast<std::tuple<DataPtrTs...>*>(&in_ptrs);
    return share;
}
} // namespace tos::arm