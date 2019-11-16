/**
 * This file contains the interfaces for the threading subsystem
 * for TOS.
 */

#pragma once

#include "tcb.hpp"

#include <stdint.h>
#include <tos/arch.hpp>
#include <tos/stack_storage.hpp>
#include <tos/thread.hpp>
#include <tos/types.hpp>

namespace tos {
namespace impl {
extern kern::tcb* cur_thread;
} // namespace impl

/**
 * Returns a pointer to the currently running thread.
 *
 * Returns `nullptr` if there's no active thread at the moment.
 *
 * @return pointer to the current thread
 */
inline kern::tcb* self() {
    return impl::cur_thread;
}

/**
 * If this object is passed to a tos::launch call, tos will allocate a
 * default sized stack for the thread
 */
constexpr stack_size_t alloc_stack{TOS_DEFAULT_STACK_SIZE};

/**
 * Launches a new thread with the given stack size.
 * 
 * The stack is allocated dynamically, and will be deallocated
 * upon thread exit.
 * 
 * Pass tos::alloc_stack to allocate a default sized stack.
 *
 * @tparam FuncT type of the callable object
 * @tparam ArgTs type of the arguments to the callable object
 * @param stack_size size of the stack to be allocated for the thread.
 * @param func callable object to run in the new thread
 * @param args arguments to be passed to the function
 * @return reference to the control block of the new thread
 */
template<class FuncT, class... ArgTs>
auto& launch(stack_size_t stack_size, FuncT&& func, ArgTs&&... args);

/**
 * Launches a new thread in the given stack storage object
 *
 * Note that the tcb will be placed on the top of the stack object,
 * so it should be large enough to accommodate the worst case stack
 * usage plus the size of the control block
 *
 * @tparam FuncT type of the callable object
 * @tparam ArgTs type of the arguments to the callable object
 * @param stack storage for the new thread
 * @param func callable object to run in the new thread
 * @param args arguments to be passed to the function
 * @return reference to the control block of the new thread
 */
template<class FuncT, class... ArgTs, size_t StSz>
auto& launch(stack_storage<StSz>& stack, FuncT&& func, ArgTs&&... args);
} // namespace tos

#include "ft.inl"