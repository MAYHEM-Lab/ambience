#pragma once

#include <tos/memory.hpp>

namespace tos {
class backing_object;
class address_space;
struct mapping {
    backing_object* obj;
    address_space* va;

    segment vm_segment;
    memory_range obj_range;

    user_accessible allow_user;
    memory_types mem_type;

    list_node<mapping> obj_list;
    list_node<mapping> va_list;

    // Used by the backend to store its private data about this mapping.
    void* backend_data;
    // copy on write flags etc can be here? or should it be per page?
};
}