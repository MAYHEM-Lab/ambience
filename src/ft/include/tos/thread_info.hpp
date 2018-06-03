#pragma once

#include <tos/intrusive_list.hpp>
#include <setjmp.h>
#include <tos/utility.hpp>

namespace tos {
    struct thread_info
            : public list_node<thread_info>
    {
        using entry_point_t = void(*)();

        volatile entry_point_t entry;
        jmp_buf context;

    public:
        explicit thread_info(entry_point_t pt) : entry{tos::move(pt)} {}
    };
}

