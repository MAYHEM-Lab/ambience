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
        struct alignas(16) tcb
            : public list_node<tcb>
        {
            using entry_point_t = void(*)();

            uint16_t stack_sz;
            jmp_buf context;

            explicit tcb(uint16_t stack_sz)
                    : stack_sz{stack_sz} {}
        };
    }
}
