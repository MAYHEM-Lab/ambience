#pragma once

#include <memory>
#include <tos/arch.hpp>
#include <tos/memory.hpp>
#include <tos/result.hpp>

namespace tos {
struct memory_fault;
struct mapping;
struct job;

enum class memory_errors
{
    out_of_bounds,
    bad_permissions,
};

TOS_ERROR_ENUM(memory_errors);

// A backing object is responsible for handling page faults in the segments it is mapped
// to in an address space.
class backing_object {
public:
    virtual auto create_mapping(const virtual_segment& vm_segment,
                                uintptr_t obj_base,
                                tos::mapping& mapping) -> result<void> = 0;

    auto clone_mapping(const mapping& original, mapping& mapping) -> result<void> {
        return create_mapping(original.vm_segment, original.obj_base, mapping);
    }

    virtual auto free_mapping(const tos::mapping& mapping) -> result<void> = 0;

    virtual auto handle_memory_fault(const memory_fault& fault) -> result<void> = 0;

    virtual ~backing_object() = default;
};

struct memory_fault {
    mapping* map;

    bool non_resident : 1;

    virtual_address virt_addr;

    size_t access_size = 8;
    permissions access_perms;

    job* faulting_job;
};
} // namespace tos