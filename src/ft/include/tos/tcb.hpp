#pragma once

#include <tos/intrusive_list.hpp>
#include <setjmp.h>
#include <tos/utility.hpp>

namespace tos {
    enum class state
    {
        runnable,
        suspended,
        running
    };
    namespace kern
    {
        /**
         * This type represents an execution context in the system.
         */
        struct alignas(16) tcb
            : public list_node<tcb>
        {
            using entry_point_t = void(*)(void*);

            uint16_t stack_sz{};
            jmp_buf context{};
            void * user;
            entry_point_t entry;
            state s;

            explicit tcb(uint16_t stack_sz) noexcept
                    : stack_sz{stack_sz} {}
        };
    }
}
