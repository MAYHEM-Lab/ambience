#pragma once

#include "tos/memory.hpp"
#include <tos/backing_object.hpp>

namespace tos {
// Segments mapped from these objects correspond to addresses in the physical memory of
// the system and correspond to stuff like IO memory.
// There can be multiple such objects, each mapping to the different part of the physical
// address space with different permissions.
class physical_memory_backing : public backing_object {
public:
    explicit constexpr physical_memory_backing(const physical_segment& phys_seg,
                                               memory_types type)
        : m_seg{phys_seg}
        , m_type{type} {
    }

    bool handle_memory_fault([[maybe_unused]] const memory_fault& fault) override;

    constexpr auto create_mapping(const virtual_segment& vm_segment,
                                  uintptr_t obj_base,
                                  tos::mapping& mapping) -> bool override {
        if (!contains(
                m_seg.range,
                physical_range(physical_address(obj_base), vm_segment.range.size))) {
            return false;
        }

        if ((int(vm_segment.perms) & int(m_seg.perms)) != int(vm_segment.perms)) {
            return false;
        }

        mapping.obj = this;
        mapping.vm_segment = vm_segment;
        mapping.obj_base = obj_base;
        mapping.mem_type = m_type;

        return true;
    }

    constexpr auto free_mapping(const tos::mapping& mapping) -> bool override {
        return true;
    }

private:
    physical_segment m_seg;
    memory_types m_type;
};
} // namespace tos