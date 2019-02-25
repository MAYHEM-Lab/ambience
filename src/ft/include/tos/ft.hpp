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