#pragma once

#include <tos/backing_object.hpp>

namespace tos {
// Segments mapped from these objects correspond to addresses in the physical memory of
// the system and correspond to stuff like IO memory.
// There can be multiple such objects, each mapping to the different part of the physical
// address space with different permissions.
class physical_memory_backing : public backing_object {
public:
    explicit physical_memory_backing(const segment& phys_seg, memory_types type)
        : m_seg{phys_seg}
        , m_type{type} {
    }

    bool handle_memory_fault([[maybe_unused]] const memory_fault& fault) override;

    auto create_mapping(const segment& vm_segment,
                        const memory_range& obj_range,
                        tos::mapping& mapping) -> bool override;

private:
    segment m_seg;
    memory_types m_type;
};
} // namespace tos