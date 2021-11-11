#pragma once

#include <memory>
#include <tos/arch.hpp>
#include <tos/memory.hpp>

namespace tos {
struct memory_fault;
struct mapping;
struct job;

// Might be anonymous, a device or an abstract object.
// A backing object is responsible for handling page faults in the segments it is mapped
// to in an address space.
class backing_object {
public:
    virtual auto create_mapping(const virtual_segment& vm_segment,
                                const memory_range& obj_range,
                                tos::mapping& mapping) -> bool = 0;

    auto clone_mapping(const mapping& original, mapping& mapping) -> bool {
        return create_mapping(original.vm_segment, original.obj_range, mapping);
    }

    virtual auto handle_memory_fault(const memory_fault& fault) -> bool = 0;

    virtual ~backing_object() = default;
};

struct memory_fault {
    mapping* map;

    bool non_resident : 1;

    virtual_address virt_addr;
    permissions access_perms;

    job* faulting_job;
};
} // namespace tos