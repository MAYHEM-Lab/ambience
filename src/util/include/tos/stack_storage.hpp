//
// Created by fatih on 2/28/19.
//

#pragma once

#include <type_traits>
#include <cstdint>
#include <cstddef>
#include <tos/span.hpp>

namespace tos
{
/**
 * Denotes a stack size.
 * Used for allocating specific amount of bytes for stack storage of tasks.
 * 
 * For instance,
 * 
 *      tos::launch(stack_size_t{4096}, foo);
 * 
 * launches a thread with 4K of stack.
 */ 
struct stack_size_t
{
    uint16_t sz;
};

/**
 * Objects of this type is used for launching threads with stacks allocated
 * at compile time (more precisely, at link time).
 * 
 * For instance,
 *      
 *      stack_storage<4096> stack_store;
 * 
 *      tos::launch(stack_store, foo);
 * 
 * launches a thread with 4K of stack that's statically allocated.
 * 
 * Care must be taken to ensure no two threads are ever launched on the
 * same stack storage object.
 */
template <std::size_t Len>
struct stack_storage
{
    using StorageT = std::aligned_storage_t<Len, alignof(std::max_align_t)>;
    StorageT m_storage;
    operator span<uint8_t>() {
        return tos::raw_cast<uint8_t>(tos::monospan(m_storage));
    }
};
} // namespace tos