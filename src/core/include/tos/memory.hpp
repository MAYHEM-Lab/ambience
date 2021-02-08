//
// Created by fatih on 7/12/18.
//

#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <tos/intrusive_list.hpp>
#include <tos/intrusive_ptr.hpp>
#include <tos/span.hpp>

namespace tos {
/**
 * This type represents a contiguous region in a memory.
 * The memory in which the addresses in the range may not be addressable directly by the
 * current processor, hence uintptr_t rather than void*.
 */
struct memory_range {
    std::uintptr_t base;
    std::ptrdiff_t size;

    [[nodiscard]] constexpr std::uintptr_t end() const {
        return base + size;
    }
};

/**
 * Returns whether the given big memory region entirely contains the given
 * small memory region.
 * @param big memory region to see if it contains the other one
 * @param small memory region to see if it's contained by the other one
 * @return whether the big region contains the small region
 */
constexpr bool contains(const memory_range& big, const memory_range& small) {
    return big.base <= small.base && big.end() >= small.end();
}

constexpr bool contains(const memory_range& range, uintptr_t addr) {
    return range.base <= addr && addr < range.end();
}

constexpr std::optional<memory_range> intersection(const memory_range& a,
                                                   const memory_range& b) {
    memory_range res{};
    res.base = std::max(a.base, b.base);
    auto end = std::min(a.end(), b.end());
    if (end <= res.base) {
        return {};
    }
    res.size = end - res.base;
    return res;
}

enum class permissions : uint8_t
{
    none,
    read = 1,
    write = 2,
    read_write = 3,
    execute = 4,
    read_execute = 5,
    all = 7
};

enum class memory_types
{
    // RAM, cachable
    normal = 1,

    // MMIO etc, not cached
    device = 2
};

enum class user_accessible
{
    no,
    yes
};

struct segment {
    memory_range range;
    permissions perms;
};

namespace default_segments {
memory_range image();
memory_range data();
memory_range text();
memory_range rodata();
memory_range bss();
} // namespace default_segments

struct mapping;
struct fault_frame;

struct job;
struct memory_fault {
    const mapping* map;

    bool non_resident : 1;

    uintptr_t virt_addr;
    permissions access_perms;

    job* faulting_job;
    fault_frame* frame;
};

// Might be anonymous, a device or an abstract object.
// A backing object is responsible for handling page faults in the segments it is mapped
// to in an address space.
class backing_object : public ref_counted<backing_object> {
public:
    virtual auto create_mapping(const segment& vm_segment, const memory_range& obj_range)
        -> std::unique_ptr<mapping> = 0;
    virtual auto handle_memory_fault(const memory_fault& fault) -> void = 0;

    virtual ~backing_object() = default;
};

class anonymous_backing : public backing_object {};

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

    void handle_memory_fault([[maybe_unused]] const memory_fault& fault) override {
    }

    std::unique_ptr<mapping> create_mapping(const segment& vm_segment,
                                            const memory_range& obj_range) override;

private:
    segment m_seg;
    memory_types m_type;
};

struct vm_backend {
public:
    virtual bool do_map(const mapping&) = 0;
    virtual bool
    mark_resident(const mapping&, const memory_range& virt_range, bool resident) = 0;
    virtual ~vm_backend() = 0;
};

class virtual_address_space;
struct mapping {
    intrusive_ptr<backing_object> obj;
    virtual_address_space* va;

    segment vm_segment;
    memory_range obj_range;

    memory_types mem_type;

    list_node<mapping> obj_list;
    list_node<mapping> va_list;

    // Used by the backend to store its private data about this mapping.
    void* backend_data;
    // copy on write flags etc can be here? or should it be per page?
};

class virtual_address_space {
public:
    void create_mapping();

private:
    intrusive_list<mapping, through_member<&mapping::va_list>> m_mappings;
};
} // namespace tos