#pragma once

#include <setjmp.h>
#include <stdlib.h>
#include <string.h>
#include <tos/tcb.hpp>
#include <tos/scheduler.hpp>

#include <tos/interrupt.hpp>
#include <tos/new.hpp>
#include <tos/memory.hpp>
#include <tos/debug.hpp>
#include <tos/compiler.hpp>

namespace tos {
    namespace this_thread {
        inline thread_id_t get_id()
        {
            if (!impl::cur_thread) return {static_cast<uintptr_t>(-1)};
            return { reinterpret_cast<uintptr_t>(impl::cur_thread) };
        }
    }

    inline thread_id_t launch(kern::tcb::entry_point_t e, void* arg)
    {
        constexpr size_t stack_size = 2048;
        auto params = thread_params()
                .add<tags::stack_ptr_t>(tos_stack_alloc(stack_size))
                .add<tags::stack_sz_t>(stack_size)
                .add<tags::entry_pt_t>(e)
                .add<tags::argument_t>(arg);
        return launch(params);
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
            if (setjmp(impl::cur_thread->context)==(int) return_codes::saved) {
                kern::make_runnable(*impl::cur_thread);
                switch_context(sched.main_context, return_codes::yield);
            }
        }
    }

    [[noreturn]]
    inline void thread_exit()
    {
        kern::disable_interrupts();

        // no need to save the current context, we'll exit

        switch_context(sched.main_context, return_codes::do_exit);
    }

    inline thread_id_t launch(launch_params params)
    {
        return sched.start(params);
    }

    namespace kern
    {
        inline void suspend_self()
        {
            //tos_debug_print("suspend %p\n", impl::cur_thread);

            if (setjmp(impl::cur_thread->context)==(int) return_codes::saved) {
                switch_context(sched.main_context, return_codes::suspend);
            }
        }

        inline exit_reason schedule()
        {
            return sched.schedule();
        }

        template <class FunT, class... Args>
        struct storage
        {
        public:
            template <class FunU, class... ArgUs>
            storage(FunU&& fun, ArgUs&&... args)
                : m_fun{tos::std::forward<FunU>(fun)},
                  m_args{tos::std::forward<ArgUs>(args)...} {}

            auto& get_entry() { return m_fun; }
            auto& get_args() { return m_args; }

        private:
            FunT m_fun;
            tos::std::tuple<Args...> m_args;
        };

        using raw_store = storage<void(*)(void*), void*>;

        template <class FunT, class... Args>
        inline thread_id_t launch_with_args(FunT&& fun, Args&&... args)
        {
            using namespace tos::std;

            using fun_t = remove_reference_t <FunT>;
            using tuple_t = tuple<Args...>;

            tuple_t args_tup { forward<Args>(args)... };

            auto wrapper = [f = fun_t(forward<FunT>(fun))] (tuple_t* t) {
                tos::apply(f, *t);
            };


        }

        inline thread_id_t scheduler::start(launch_params params)
        {
            const auto st_size = get<tags::stack_sz_t>(params);
            const auto stack = static_cast<char*>(get<tags::stack_ptr_t>(params));

            const auto stack_top = stack + st_size;

            const auto t_ptr = stack_top - sizeof(tcb);

            tcb::entry_point_t entry = get<tags::entry_pt_t>(params);
            void* user_arg = get<tags::argument_t>(params);
            auto thread = new (t_ptr) super_tcb<raw_store>(st_size - (sizeof(super_tcb<raw_store>) - sizeof(tcb)), raw_store{ entry, user_arg });

            // New threads are runnable by default.
            run_queue.push_back(*thread);
            num_threads++;

            kern::disable_interrupts();
            if (setjmp(thread->context)==(int) return_codes::saved) {
                kern::enable_interrupts();
                return { reinterpret_cast<uintptr_t>(thread) };
            }

            /**
             * this is the actual entry point of the thread.
             * will be called when scheduled
             *
             * set the stack pointer so the new thread will have an
             * independent execution context
             */
            tos_set_stack_ptr(reinterpret_cast<char*>(impl::cur_thread));

            kern::enable_interrupts();

            using tcb_t = super_tcb<raw_store>;
            tos::apply(
                    static_cast<tcb_t *>(impl::cur_thread)->get_entry(),
                    static_cast<tcb_t *>(impl::cur_thread)->get_args());
            this_thread::exit(nullptr);
        }

        inline void busy() { sched.busy++; }
        inline void unbusy() { sched.busy--; }

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

                        switch_context(impl::cur_thread->context, return_codes::scheduled);
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

    inline void this_thread::exit(void*)
    {
        thread_exit();
    }
}


