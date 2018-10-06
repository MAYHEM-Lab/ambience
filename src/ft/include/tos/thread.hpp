//
// Created by fatih on 10/5/18.
//

#pragma once

#include <tos/types.hpp>
#include <tos/ct_map.hpp>
#include <stddef.h>
#include <tos/tcb.hpp>

namespace tos
{
    namespace tags
    {
        struct stack_ptr_t {};
        struct entry_pt_t {};
        struct stack_sz_t {};
        struct argument_t {};
    }

    using launch_params =
    ct_map<
        base_key_policy,
        el_t<tags::stack_ptr_t, void* const &>,
        el_t<tags::stack_sz_t, const size_t&>,
        el_t<tags::entry_pt_t, const kern::tcb::entry_point_t&>,
        el_t<tags::argument_t, void* const&>
    >;

    namespace this_thread {
        /**
         * Returns a platform dependent identifier of the current task.
         *
         * Returns a defined, but unspecified value if this function is
         * called from a non-thread context.
         */
        thread_id_t get_id();

        /**
         * Give control of the CPU back to the scheduler. This will leave
         * the thread in a runnable state.
         *
         * Behaviour is undefined if this function is called from a
         * non-thread context.
         *
         * A blocking mechanism should always be preferred over yielding
         * in a busy wait.
         */
        void yield();

        /**
         * Causes the current thread to exit, stopping the execution
         * and destroying the stack.
         *
         * No other resource will be returned to the OS through an
         * exit. Thus, it must be used with care. Prefer exiting a
         * thread through returning from the entry point function.
         *
         * Behaviour is undefined if this function is called from a
         * non-thread context.
         */
        [[noreturn]]
        void exit(void* res = nullptr);
    }
}
