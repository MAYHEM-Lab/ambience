#pragma once

#include <tos/intrusive_list.hpp>
#include <setjmp.h>

namespace tos {
    struct thread_info
            : public list_node<thread_info>
    {
        void (* entry)();
        jmp_buf context;
    };
}

