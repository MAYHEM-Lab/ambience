#pragma once

#include <tos/intrusive_list.hpp>
#include <setjmp.h>
#include <tos/utility.hpp>

namespace tos {
    /**
     * This type represents an execution context in the system.
     */
    struct tcb
            : public list_node<tcb>
    {
        using entry_point_t = void(*)();

        volatile entry_point_t entry;
        jmp_buf context;

        explicit tcb(entry_point_t pt) : entry{tos::move(pt)} {}
    };
}

