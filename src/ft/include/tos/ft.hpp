#pragma once

#include <stdint.h>
#include "thread_info.hpp"

namespace tos {
    using entry_t = void (*)();

    struct task_id_t
    {
        uintptr_t id;
    };

    void launch(entry_t) __attribute__((visibility("default")));

    void schedule();

    namespace this_thread {
        task_id_t get_id();

        void yield();

        void exit(void* res = nullptr);
    }

    uint8_t runnable_count();

    namespace impl {
        extern thread_info* cur_thread;
    }

    inline thread_info* self()
    {
        return impl::cur_thread;
    }
}

namespace tos
{
    namespace this_thread
    {
        inline task_id_t get_id()
        {
            if (!impl::cur_thread) return {static_cast<uintptr_t>(-1)};
            return {reinterpret_cast<uintptr_t>(impl::cur_thread) };
        }
    }
}

