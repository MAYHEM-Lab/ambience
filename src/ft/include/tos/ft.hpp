#pragma once

#include <stdint.h>
#include "thread_info.hpp"

namespace tos {
    /**
     * This struct represents a unique identifier for every
     * **running** task in the system.
     */
    struct thread_id_t
    {
        uintptr_t id;
    };

    void launch(thread_info::entry_point_t);

    void schedule();

    namespace this_thread {
        /**
         * Returns the identifier of the current task
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
        extern thread_info* cur_thread;
    }

    inline thread_info* self()
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

