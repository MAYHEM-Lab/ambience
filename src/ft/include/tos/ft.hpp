#pragma once

#include <stdint.h>
#include "tcb.hpp"
#include <tos/ct_map.hpp>

namespace tos {
    /**
     * This struct represents a unique identifier for every
     * **running** task in the system.
     */
    struct thread_id_t { uintptr_t id; };

    thread_id_t launch(tcb::entry_point_t);

    namespace tags
    {
        struct stack_ptr_t {};
        struct entry_pt_t {};
        struct stack_sz_t {};
    }

    static constexpr tags::stack_ptr_t stack_ptr{};
    static constexpr tags::entry_pt_t entry_pt{};
    static constexpr tags::stack_sz_t stack_sz{};

    using launch_params =
        ct_map<
            base_key_policy,
            el_t<tags::stack_ptr_t, void* const &>,
            el_t<tags::stack_sz_t, const size_t&>,
            el_t<tags::entry_pt_t, const tcb::entry_point_t&>
        >;

    inline constexpr auto thread_params() { return ct_map<base_key_policy>{}; }

    thread_id_t launch(launch_params);

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

    exit_reason schedule();

    namespace this_thread {
        /**
         * Returns a platform dependent identifier of the current task
         */
        thread_id_t get_id();

        /**
         * Give control of the CPU back to the scheduler
         */
        void yield();

        [[noreturn]]
        void exit(void* res = nullptr);
    }

    enum class return_codes : uint8_t;

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

