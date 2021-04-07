#include <tos/address_space.hpp>
#include <tos/arch.hpp>
#include <tos/mapping.hpp>
#include <tos/physical_memory_backing.hpp>

namespace tos {
bool physical_memory_backing::create_mapping(const segment& vm_segment,
                                             const memory_range& obj_range,
                                             tos::mapping& mapping) {
    if (!contains(m_seg.range, obj_range)) {
        return false;
    }

    if ((int(vm_segment.perms) & int(m_seg.perms)) != int(vm_segment.perms)) {
        return false;
    }

    mapping.obj = intrusive_ptr<backing_object>(this);
    mapping.vm_segment = vm_segment;
    mapping.obj_range = obj_range;
    mapping.mem_type = m_type;

    return true;
}

bool physical_memory_backing::handle_memory_fault(const memory_fault& fault) {
    // This virtual region is backed directly by physical memory, so we can just mark it
    // as resident.

    // We don't want to map the entire world just for a single word.
    // Get the containing fragment, and map only that.
    auto fault_fragment = fault.map->va->m_backend->containing_fragment(
        {.base = fault.virt_addr, .size = sizeof(uintmax_t)});

    fault.map->va->m_backend->mark_resident(
        *fault.map, fault_fragment, reinterpret_cast<void*>(fault.map->obj_range.base));

    return true;
}
} // namespace tos