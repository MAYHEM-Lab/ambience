#include <tos/address_space.hpp>
#include <tos/arch.hpp>
#include <tos/mapping.hpp>
#include <tos/physical_memory_backing.hpp>

namespace tos {
bool physical_memory_backing::handle_memory_fault(const memory_fault& fault) {
    // This virtual region is backed directly by physical memory, so we can just mark it
    // as resident.

    // We don't want to map the entire world just for a single word.
    // Get the containing fragment, and map only that.
    auto fault_fragment = fault.map->va->containing_fragment(
        {.base = fault.virt_addr, .size = sizeof(uintmax_t)});

    fault.map->va->mark_resident(
        *fault.map, fault_fragment, physical_address{fault.map->obj_range.base});

    return true;
}
} // namespace tos