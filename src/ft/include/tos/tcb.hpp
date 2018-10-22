#pragma once

#include <tos/intrusive_list.hpp>
#include <setjmp.h>
#include <tos/utility.hpp>
#include <tos/arch.hpp>

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

            jmp_buf context;

            char* get_stack_base()
            {
                return reinterpret_cast<char*>(this) + sizeof(*this) - stack_sz;
            }

            explicit tcb(uint16_t stack_size) noexcept
                    : stack_sz{stack_size} {}

            ~tcb()
            {
                tos_stack_free(get_stack_base());
            }

        private:
            uint16_t stack_sz{};
        };

        template <class ArgT>
        struct super_tcb : tcb, ArgT
        {
            template <class ArgU>
            super_tcb(uint16_t stack_sz, ArgU&& arg)
                : tcb(stack_sz), ArgT(tos::std::forward<ArgU>(arg)) {}

            using ArgT::get_entry;
            using ArgT::get_args;
        };
    }
}
