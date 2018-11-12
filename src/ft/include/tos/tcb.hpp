#pragma once

#include <tos/intrusive_list.hpp>
#include <setjmp.h>
#include <utility>
#include <tos/utility.hpp>
#include <tos/arch.hpp>
#include <cstddef>
#include <algorithm>

namespace tos {
    namespace kern
    {
        /**
         * This type represents an execution context in the system.
         *
         * It's expected that a concrete task descriptor type will
         * extend this class to implement required functionality such
         * as starting threads or passing arguments.
         */
        struct alignas(alignof(std::max_align_t)) tcb
            : public list_node<tcb>, public non_copy_movable
        {
            /**
             * Constructs a new tcb object with the given stack offset
             * @param stack_off
             */
            explicit tcb(uint16_t stack_off) noexcept
                    : m_tcb_off{stack_off} {}

            /**
             * This function computes the beginning of the memory block
             * of the task this tcb belongs to.
             *
             * @return pointer to the task's base
             */
            char* get_task_base()
            {
                return reinterpret_cast<char*>(this) - m_tcb_off;
            }

            /**
             * Returns a reference to the context of the task.
             *
             * The function can either be called to store the current
             * context, or to resume execution from the context.
             *
             * @return execution context of the task
             */
            jmp_buf& get_context()
            {
                return m_context;
            }

            /**
             * The threading subsystem does not know about the concrete time
             * at the destruction of a task, so the destructor must be virtual
             * in order to properly destroy the concrete task descriptor.
             */
            virtual ~tcb() = 0;
        private:
            uint16_t m_tcb_off; // we store the offset of this object from the task base
            jmp_buf m_context;
        };

        inline tcb::~tcb() = default;
    } // namespace kern
} //namespace tos
