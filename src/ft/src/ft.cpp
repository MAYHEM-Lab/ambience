#include <setjmp.h>
#include <stdlib.h>
#include <tos/ft.hpp>
#include <tos/tcb.hpp>
#include <tos/scheduler.hpp>

#include <tos_arch.hpp>
#include <tos/interrupt.hpp>
#include <tos/new.hpp>
#include <tos/memory.hpp>

namespace tos {
    namespace kern {
        struct scheduler
        {
            jmp_buf main_context{};
            int8_t  num_threads = 0;
            uint8_t busy = 0;
            intrusive_list<tcb> run_queue;

            thread_id_t start(launch_params);

            exit_reason schedule();
        };
    }
}

namespace tos
{
    [[noreturn]] static void switch_context(jmp_buf& j, return_codes rc)
    {
        longjmp(j, static_cast<int>(rc));
    }
}

namespace tos {
    static kern::scheduler sched;
    namespace impl
    {
        kern::tcb* cur_thread = nullptr;
    }

    namespace this_thread
    {
        void yield()
        {
            tos::int_guard ig;
            if (setjmp(impl::cur_thread->context)==(int) return_codes::saved) {
                kern::make_runnable(*impl::cur_thread);
                switch_context(sched.main_context, return_codes::yield);
            }
        }
    }

    [[noreturn]]
    static void thread_exit()
    {
        kern::disable_interrupts();

        // no need to save the current context, we'll exit

        switch_context(sched.main_context, return_codes::do_exit);
    }

    thread_id_t launch(launch_params params)
    {
        return sched.start(params);
    }

    namespace kern
    {
        void suspend_self()
        {
            if (setjmp(impl::cur_thread->context)==(int) return_codes::saved) {
                switch_context(sched.main_context, return_codes::suspend);
            }
        }

        exit_reason schedule()
        {
            return sched.schedule();
        }

        inline thread_id_t scheduler::start(launch_params params)
        {
            const auto stack = static_cast<char*>(get<tags::stack_ptr_t>(params));
            const auto t_ptr = stack + get<tags::stack_sz_t>(params) - sizeof(tcb);

            auto thread = new (t_ptr) tcb(get<tags::entry_pt_t>(params), get<tags::stack_sz_t>(params));

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
            impl::cur_thread->entry();
            this_thread::exit(nullptr);
        }

        void busy() { sched.busy++; }
        void unbusy() { sched.busy--; }

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
                    auto stack_ptr = reinterpret_cast<char*>(impl::cur_thread)
                            + sizeof(tcb) - impl::cur_thread->stack_sz;
                    std::destroy_at(impl::cur_thread);
                    tos_stack_free(stack_ptr);
                    num_threads--;
                }
                default:
                    break;
                }

                impl::cur_thread = nullptr;
            }
        }

        void make_runnable(tcb& t)
        {
            sched.run_queue.push_back(t);
        }
    }

    void this_thread::exit(void*)
    {
        thread_exit();
    }
}


