/**
 * This file contains the interfaces for the threading subsystem
 * for TOS.
 */

#pragma once

#include <stdint.h>
#include "tcb.hpp"
#include <tos/ct_map.hpp>
#include <tos/arch.hpp>

namespace tos {
    /**
     * This struct represents a unique identifier for every
     * **running** task in the system.
     */
    struct thread_id_t { uintptr_t id; };

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

    /**
     * This enumeration denotes why the scheduler returned
     */
    enum class exit_reason
    {
        /**
         * All threads are blocked, however there are
         * busy threads. System should wait for any interrupt.
         */
        idle,
        /**
         * All threads are blocked, and the system isn't busy.
         * We can go to the lowest possible power state.
         */
        power_down,
        /**
         * All threads exited. Usually denotes a bug, system
         * should restart.
         */
        restart,
        /**
         * Threading subsystem yields the CPU back to the
         * architecture. This is useful for architectures that
         * require periodic control over CPU.
         *
         * If the architecture doesn't mandate such a requirement
         * this value may never be returned.
         */
        yield
    };

    namespace kern
    {
        /**
         * This function starts the thread execution subsystem. It
         * will not return as long as there exists a runnable thread
         * in the system.
         *
         * When all threads block, it will return a reason explaining
         * why the return happened.
         *
         * Depending on the reason, implementations should enter a
         * power saving state accordingly.
         *
         * Behaviour is undefined if this function is called from a
         * thread context.
         *
         * @return exit reason
         */
        exit_reason schedule();
    }

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

    size_t runnables();
}

namespace tos {
    namespace this_thread {
        inline thread_id_t get_id()
        {
            if (!impl::cur_thread) return {static_cast<uintptr_t>(-1)};
            return { reinterpret_cast<uintptr_t>(impl::cur_thread) };
        }
    }

    inline thread_id_t launch(kern::tcb::entry_point_t e, void* arg)
    {
        constexpr size_t stack_size = 3072;
        auto params = thread_params()
            .add<tags::stack_ptr_t>(tos_stack_alloc(stack_size))
            .add<tags::stack_sz_t>(stack_size)
            .add<tags::entry_pt_t>(e)
            .add<tags::argument_t>(arg);
        return launch(params);
    }

    enum class return_codes : uint8_t
    {
        saved = 0,
        /**
         * the running thread yielded
         */
        yield,
        /**
         * the running thread has been suspended
         */
        suspend,
        /**
         * a thread exited
         */
        do_exit,
        /**
         * this thread was assigned the cpu
         */
        scheduled
    };
}

