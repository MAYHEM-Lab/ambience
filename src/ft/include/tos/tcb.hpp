#pragma once

#include <tos/intrusive_list.hpp>
#include <setjmp.h>
#include <tos/utility.hpp>

namespace tos {
    namespace kern
    {
        /**
         * This type represents an execution context in the system.
         */
        struct tcb
            : public list_node<tcb>
        {
            using entry_point_t = void(*)();

            entry_point_t entry;
            uint16_t stack_sz;
            jmp_buf context;

            explicit tcb(entry_point_t pt, uint16_t stack_sz)
                    : entry{std::move(pt)}, stack_sz{stack_sz} {}
        };
    }
}
