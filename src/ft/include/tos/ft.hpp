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
    thread_id_t launch(tcb::entry_point_t);

    namespace tags
    {
        struct stack_ptr_t {};
        struct entry_pt_t {};
        struct stack_sz_t {};
    }

    using launch_params =
        ct_map<
            base_key_policy,
            el_t<tags::stack_ptr_t, void* const &>,
            el_t<tags::stack_sz_t, const size_t&>,
            el_t<tags::entry_pt_t, const tcb::entry_point_t&>
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
        restart
    };

    namespace kern
    {
        exit_reason schedule();
    }

    namespace this_thread {
        /**
         * Returns a platform dependent identifier of the current task.
         */
        thread_id_t get_id();

        /**
         * Give control of the CPU back to the scheduler. This will leave
         * the thread in a runnable state.
         */
        void yield();

        /**
         * Causes the current thread to exit, stopping the execution
         * and destroying the stack.
         *
         * No other resource will be returned to the OS through an
         * exit. Thus, it must be used with care. Prefer exiting a
         * thread through returning from the entry point function.
         */
        [[noreturn]]
        void exit(void* res = nullptr);
    }


    namespace impl {
        extern tcb* cur_thread;
    }

    /**
     * Returns a pointer to the currently running thread.
     *
     * Returns `nullptr` if there's no active thread at the moment.
     *
     * @return pointer to the current thread
     */
    inline tcb* self()
    {
        return impl::cur_thread;
    }
}

namespace tos {
    namespace this_thread {
        inline thread_id_t get_id()
        {
            if (!impl::cur_thread) return {static_cast<uintptr_t>(-1)};
            return {reinterpret_cast<uintptr_t>(impl::cur_thread)};
        }
    }

    inline thread_id_t launch(tcb::entry_point_t e)
    {
        constexpr size_t stack_size = 512;
        auto params = thread_params()
            .add<tags::stack_ptr_t>(tos_stack_alloc(stack_size))
            .add<tags::stack_sz_t>(stack_size)
            .add<tags::entry_pt_t>(e);
        return launch(params);
    }

    enum class return_codes : uint8_t
    {
        saved = 0,
        /**
         * a running thread yielded
         */
        yield,
        /**
         * a running thread is waiting on something
         */
        suspend,
        /**
         * a thread exited
         */
        do_exit,
        /**
         * the thread was assigned the cpu
         */
        scheduled
    };
}

