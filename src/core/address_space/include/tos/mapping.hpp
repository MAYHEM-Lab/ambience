#pragma once

#include <tos/core/arch_fwd.hpp>
#include <tos/intrusive_list.hpp>
#include <tos/memory.hpp>

namespace tos {
class backing_object;
struct mapping {
    backing_object* obj;
    cur_arch::address_space* va;

    virtual_segment vm_segment; // Where this mapping is in the address space.
    uintptr_t obj_base;

    user_accessible allow_user : 1;
    memory_types mem_type : 2;
    bool owned : 1 = false; // Whether the mapping should be freed by its address space.

    list_node<mapping> obj_list;
    list_node<mapping> va_list;
};
} // namespace tos