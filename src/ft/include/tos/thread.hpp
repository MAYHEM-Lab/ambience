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
    } // namespace tags

    using launch_params =
    ct_map<
        base_key_policy,
        el_t<tags::stack_ptr_t, void* const &>,
        el_t<tags::stack_sz_t, const size_t&>,
        el_t<tags::entry_pt_t, void(* const&)(void*)>,
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
         * This function yields the control back to the kernel, however,
         * the thread will not be placed back into the runnable queue,
         * which blocks this thread forever in a non-resumable way.
         *
         * The thread will NOT save it's last state, thus it CANNOT be
         * resumed, even with a handle to the thread!
         *
         * If you'd like to implement your own waiting semantics, you should
         * either build on top of `waitable` or use `kern::suspend_self`
         * instead of this.
         *
         * This is a dangerous function and should probably not be used!
         */
        [[noreturn]]
        void block_forever();

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
    } // namespace this_thread
} //namespace tos
