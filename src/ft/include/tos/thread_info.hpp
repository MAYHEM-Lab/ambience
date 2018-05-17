#pragma once

#include <tos/intrusive_list.hpp>
#include <setjmp.h>

namespace tos {
    struct thread_info
            : public list_node<thread_info>
    {
        using entry_point_t = void(*)();

        entry_point_t entry;
        jmp_buf context;
    };
}

