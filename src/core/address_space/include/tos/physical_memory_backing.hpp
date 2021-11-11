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
    explicit constexpr physical_memory_backing(const physical_segment& phys_seg, memory_types type)
        : m_seg{phys_seg}
        , m_type{type} {
    }

    bool handle_memory_fault([[maybe_unused]] const memory_fault& fault) override;

    constexpr auto create_mapping(const virtual_segment& vm_segment,
                                  const memory_range& obj_range,
                                  tos::mapping& mapping) -> bool override {
        if (!contains(to_memory_range(m_seg.range), obj_range)) {
            return false;
        }

        if ((int(vm_segment.perms) & int(m_seg.perms)) != int(vm_segment.perms)) {
            return false;
        }

        mapping.obj = this;
        mapping.vm_segment = vm_segment;
        mapping.obj_range = obj_range;
        mapping.mem_type = m_type;

        return true;
    }

private:
    physical_segment m_seg;
    memory_types m_type;
};
} // namespace tos