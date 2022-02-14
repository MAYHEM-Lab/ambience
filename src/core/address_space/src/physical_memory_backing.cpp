#include <tos/address_space.hpp>
#include <tos/arch.hpp>
#include <tos/mapping.hpp>
#include <tos/physical_memory_backing.hpp>

namespace tos {
result<void> physical_memory_backing::handle_memory_fault(const memory_fault& fault) {
    // This virtual region is backed directly by physical memory, so we can just mark it
    // as resident.

    // We don't want to map the entire world just for a single word.
    // Get the containing fragment, and map only that.
    auto fault_fragment =
        fault.map->va->containing_fragment({fault.virt_addr, sizeof(uintmax_t)});

    fault.map->va->mark_resident(*fault.map,
                                 {fault_fragment, fault.map->vm_segment.perms},
                                 physical_address{fault.map->obj_base});

    return {};
}
} // namespace tos