#pragma once

#include <setjmp.h>
#include <stdlib.h>
#include <string.h>
#include <tos/tcb.hpp>
#include <tos/scheduler.hpp>

#include <tos/interrupt.hpp>
#include <memory>
#include <new>
#include <tos/debug.hpp>
#include <tos/compiler.hpp>
#include <tos/span.hpp>

namespace tos {
    namespace this_thread {
        inline thread_id_t get_id()
        {
            if (!impl::cur_thread) return {static_cast<uintptr_t>(-1)};
            return { reinterpret_cast<uintptr_t>(impl::cur_thread) };
        }
    }

    enum class return_codes : uint8_t
    {
        saved = 0,
        /**
         * the running thread yielded
         */
        yield,
        /**
         * the running thread has been suspended
         */
        suspend,
        /**
         * a thread exited
         */
        do_exit,
        /**
         * this thread was assigned the cpu
         */
        scheduled
    };
}

namespace tos
{
    [[noreturn]] inline void switch_context(jmp_buf& j, return_codes rc)
    {
        longjmp(j, static_cast<int>(rc));

        __builtin_unreachable();
    }
}

namespace tos {
    extern kern::scheduler sched;

    namespace this_thread
    {
        inline void yield()
        {
            tos::int_guard ig;
            if (setjmp(impl::cur_thread->get_context())==(int) return_codes::saved) {
                kern::make_runnable(*impl::cur_thread);
                switch_context(sched.main_context, return_codes::yield);
            }
        }

        inline void block_forever()
        {
            kern::disable_interrupts();
            switch_context(sched.main_context, return_codes::suspend);
        }
    }

    [[noreturn]]
    inline void thread_exit()
    {
        kern::disable_interrupts();

        // no need to save the current context, we'll exit

        switch_context(sched.main_context, return_codes::do_exit);
    }

    namespace kern
    {
        inline void suspend_self()
        {
            // interrupts are assumed to be disabled for this function to be called
            //tos_debug_print("suspend %p\n", impl::cur_thread);

            if (setjmp(impl::cur_thread->get_context())==(int) return_codes::saved) {
                switch_context(sched.main_context, return_codes::suspend);
            }
        }

        inline exit_reason schedule()
        {
            return sched.schedule();
        }

        template <class FunT, class... Args>
        struct super_tcb : tcb
        {
            template <class FunU, class... ArgUs>
            super_tcb(uint16_t stk_sz, FunU&& fun, ArgUs&&... args)
                    : m_tcb_off(stk_sz - sizeof(super_tcb)),
                      m_fun{std::forward<FunU>(fun)},
                      m_args{std::forward<ArgUs>(args)...} {}

            // This must not be inlined so that we don't mess up with the compiler's
            // stack allocation assumptions.
            void NORETURN NO_INLINE start()
            {
                kern::enable_interrupts();

                std::apply(m_fun, m_args);

                this_thread::exit(nullptr);
            }
            
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

            ~super_tcb() final
            {
                tos_stack_free(get_task_base());
            }

        private:
            uint16_t m_tcb_off; // we store the offset of this object from the task base
            FunT m_fun;
            std::tuple<Args...> m_args;
        };

        template <class FuncT, class... ArgTs>
        using lambda_task = super_tcb<std::decay_t<std::remove_reference_t <FuncT>>, std::decay_t<std::remove_reference_t <ArgTs>>...>;

		template <class FuncT, class... ArgTs>
        lambda_task<FuncT, ArgTs...>&
        prep_lambda_layout(tos::span<char> task_data, FuncT&& func, ArgTs&&... args)
        {
            const auto stack_top = task_data.end();

            const auto t_ptr = stack_top - sizeof(lambda_task<FuncT, ArgTs...>);

            auto thread = new (t_ptr) lambda_task<FuncT, ArgTs...>(
                    task_data.size(),
                    std::forward<FuncT>(func),
                    std::forward<ArgTs>(args)...);

            return *thread;
        }

        template <class TaskT>
        inline thread_id_t scheduler::start(TaskT& t)
        {
            static_assert(std::is_base_of<tcb, TaskT>{}, "Tasks must inherit from tcb class!");

            // New threads are runnable by default.
            run_queue.push_back(t);
            num_threads++;

            kern::disable_interrupts();
            if (setjmp(t.get_context())==(int) return_codes::saved) {
                kern::enable_interrupts();
                return { reinterpret_cast<uintptr_t>(static_cast<tcb*>(&t)) };
            }

            /**
             * this is the actual entry point of the thread.
             * will be called when scheduled
             *
             * set the stack pointer so the new thread will have an
             * independent execution context
             */
            tos_set_stack_ptr(reinterpret_cast<char*>(impl::cur_thread));

            static_cast<TaskT *>(impl::cur_thread)->start();

            __builtin_unreachable();
        }

        inline void busy() {
            sched.busy++;
        }

        inline void unbusy() {
            sched.busy--;
        }

        inline exit_reason scheduler::schedule()
        {
            while (true)
            {
                if (num_threads==0) {
                    // no thread left, potentially a bug
                    return exit_reason::restart;
                }

                /**
                 * We must disable interrupts before we look at the run_queue and sc.busy.
                 * An interrupt might occur between the former and the latter and we can
                 * power down even though there's something to run.
                 */
                tos::int_guard ig;
                if (run_queue.empty()) {
                    /**
                     * there's no thread to run right now
                     */

                    if (sched.busy > 0)
                    {
                        return exit_reason::idle;
                    }

                    return exit_reason::power_down;
                }

                auto why = static_cast<return_codes>(setjmp(sched.main_context));

                switch (why) {
                    case return_codes::saved:
                    {
                        impl::cur_thread = &run_queue.front();
                        run_queue.pop_front();

                        switch_context(impl::cur_thread->get_context(), return_codes::scheduled);
                    }
                    case return_codes::do_exit:
                    {
                        std::destroy_at(impl::cur_thread);
                        num_threads--;
                        break;
                    }
                    case return_codes::yield:
                    case return_codes::suspend:
                    default:
                        break;
                }

                impl::cur_thread = nullptr;
                return exit_reason::yield;
            }
        }

        inline void make_runnable(tcb& t)
        {
            sched.run_queue.push_back(t);
        }
    }

	template <class FuncT, class... ArgTs>
    inline auto& launch(tos::span<char> task_span, FuncT&& func, ArgTs&&... args)
    {
        auto& t = kern::prep_lambda_layout(task_span, std::forward<FuncT>(func), std::forward<ArgTs>(args)...);
        sched.start(t);
        return t;
    }

    template <class FuncT, class... ArgTs>
    inline auto& launch(FuncT&& func, ArgTs&&... args)
    {
        constexpr size_t stack_size = TOS_DEFAULT_STACK_SIZE;
        tos::span<char> task_span((char*)tos_stack_alloc(stack_size), stack_size);
        return launch(task_span, std::forward<FuncT>(func), std::forward<ArgTs>(args)...);
    }

    inline void this_thread::exit(void*)
    {
        thread_exit();
    }
}


