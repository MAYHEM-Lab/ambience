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
            jmp_buf m_context;
        };

        inline tcb::~tcb() = default;
    } // namespace kern
} //namespace tos
