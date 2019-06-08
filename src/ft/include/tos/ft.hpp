/**
 * This file contains the interfaces for the threading subsystem
 * for TOS.
 */

#pragma once

#include <stdint.h>
#include "tcb.hpp"
#include <tos/arch.hpp>
#include <tos/types.hpp>
#include <tos/thread.hpp>
#include <tos/stack_storage.hpp>

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
    inline kern::tcb* self()
    {
        return impl::cur_thread;
    }

    /**
     * This type represents a tag to let tos know that it should allocate a
     * stack with the default size for the platform.
     */
    struct alloc_stack_t {};

    /**
     * If this object is passed to a tos::launch call, tos will allocate a
     * default sized stack for the thread
     */
    constexpr alloc_stack_t alloc_stack{};

    /**
     * Launches a new thread with a default sized stack
     *
     * @tparam FuncT type of the callable object
     * @tparam ArgTs type of the arguments to the callable object
     * @param func callable object to run in the new thread
     * @param args arguments to be passed to the function
     * @return reference to the control block of the new thread
     */
    template<class FuncT, class... ArgTs>
    auto &launch(alloc_stack_t, FuncT &&func, ArgTs &&... args);

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
    auto &launch(stack_storage<StSz> &stack, FuncT &&func, ArgTs &&... args);
} // namespace tos

#include "ft.inl"