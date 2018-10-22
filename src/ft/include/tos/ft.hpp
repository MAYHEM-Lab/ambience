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

namespace tos {

    /**
     * Launches a new thread with the given entry point function.
     *
     * Allocates stack using the deprecated `tos_allocate_stack`
     * function. Avoid using this version and prefer the overload
     * taking a launch_params object.
     *
     * @return identifier for the newly created thread
     */
    [[deprecated]]
    thread_id_t launch(kern::tcb::entry_point_t, void* arg = nullptr);

    inline constexpr auto thread_params() { return ct_map<base_key_policy>{}; }

    /**
     * Launches a new thread with the given parameters.
     *
     * Parameters must include the entry point, a stack and the size
     * of the stack for the newly created thread. See `tos::launch_params`
     * for the details the parameters.
     *
     * @param params launch parameters
     *
     * @return identifier of the new thread
     */
    thread_id_t launch(launch_params params);


    namespace impl {
        extern kern::tcb* cur_thread;
    }

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
}

#include "ft.inl"