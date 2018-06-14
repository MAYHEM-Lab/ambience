#pragma once

#include <stdint.h>
#include "tcb.hpp"

namespace tos {
    /**
     * This struct represents a unique identifier for every
     * **running** task in the system.
     */
    struct thread_id_t
    {
        uintptr_t id;
    };

    void launch(tcb::entry_point_t);

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
         * Returns a platform depentent identifier of the current task
         */
        thread_id_t get_id();

        /**
         * Give control of the CPU back to the scheduler
         */
        void yield();

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
        do_wait,
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

