#include <setjmp.h>
#include <stdlib.h>
#include <tos/ft.hpp>
#include <tos/tcb.hpp>
#include <tos/scheduler.hpp>

#include <tos/interrupt.hpp>
#include <tos/new.hpp>
#include <tos/memory.hpp>
#include <tos/debug.hpp>

namespace tos {
    namespace kern {
        struct scheduler
        {
            jmp_buf main_context{};
            int8_t  num_threads = 0;
            uint8_t busy = 0;
            intrusive_list<tcb> run_queue;
            int8_t runnable = 0;

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

    size_t runnables() {
        return sched.runnable;
    }

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
            //tos_debug_print("suspend %p\n", impl::cur_thread);

            if (setjmp(impl::cur_thread->context)==(int) return_codes::saved) {
                impl::cur_thread->s = state::suspended;
                switch_context(sched.main_context, return_codes::suspend);
            }
        }

        exit_reason schedule()
        {
            return sched.schedule();
        }

        template <class T>
        void push(uintptr_t stack, T val)
        {
            // assumes stack grows downwards
            char* stptr = reinterpret_cast<char*>(stack);
            stptr -= sizeof val;
            memcpy(stptr, &val, sizeof val);
        }

        template <class T>
        T pop(uintptr_t stack)
        {
            // assumes stack grows downwards
            T t;
            char* stptr = reinterpret_cast<char*>(stack);
            stptr -= sizeof t;
            memcpy(&t, stptr, sizeof t);
            return t;
        }

        static constexpr uint64_t stack_guard = 0xD5E5A5D5B5E5E5F5;

        struct stack_smash_status
        {
            bool result;
            bool guard1, guard2, guard3;
            uint64_t val1, val2, val3;
        };

        stack_smash_status check_stack_smash(char* stack, uint16_t st_size)
        {
            const auto stack_top    = stack + st_size;
            const auto st_g_ptr     = stack_top - sizeof stack_guard;
            const auto t_ptr        = st_g_ptr  - sizeof(tcb);
            const auto st_g2_ptr    = t_ptr     - sizeof stack_guard;
            const auto st_g3_ptr    = stack;

            auto guard1 = *reinterpret_cast<const uint64_t*>(st_g_ptr);
            auto guard2 = *reinterpret_cast<const uint64_t*>(st_g2_ptr);
            auto guard3 = *reinterpret_cast<const uint64_t*>(st_g3_ptr);

            stack_smash_status res;
            res.guard1 = guard1 == stack_guard;
            res.guard2 = guard2 == stack_guard;
            res.guard3 = guard3 == stack_guard;
            res.val1 = guard1;
            res.val2 = guard2;
            res.val3 = guard3;
            res.result = res.guard1 && res.guard2 && res.guard3;

            return res;
        }

        inline thread_id_t scheduler::start(launch_params params)
        {
            const auto st_size = get<tags::stack_sz_t>(params);
            const auto stack = static_cast<char*>(get<tags::stack_ptr_t>(params));

            const auto stack_top    = stack + st_size;
            const auto st_g_ptr     = stack_top - sizeof stack_guard;
            const auto t_ptr        = st_g_ptr  - sizeof(tcb);
            const auto st_g2_ptr    = t_ptr     - sizeof stack_guard;
            const auto st_g3_ptr    = stack;

            tcb::entry_point_t entry = get<tags::entry_pt_t>(params);
            void* user_arg = get<tags::argument_t>(params);

            auto guard1 = new (st_g_ptr)    uint64_t(stack_guard);
            auto thread = new (t_ptr)       tcb     (st_size);
            auto guard2 = new (st_g2_ptr)   uint64_t(stack_guard);
            auto guard3 = new (st_g3_ptr)   uint64_t(stack_guard);

            thread->entry = entry;
            thread->user = user_arg;

            if (!check_stack_smash(stack, st_size).result)
            {
                tos_debug_print("Stack Guard Failure!");
                while (true);
            }

            // New threads are runnable by default.
            thread->s = state::runnable;
            run_queue.push_back(*thread);
            runnable++;
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
            tos_set_stack_ptr(reinterpret_cast<char*>(impl::cur_thread) - sizeof stack_guard);

            kern::enable_interrupts();

            impl::cur_thread->entry(impl::cur_thread->user);
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

                validate(run_queue, [](bool b, const char* r){
                    if (!b)
                    {
                        tos_debug_print("%s", r);
                        while (true);
                    }
                });

                auto why = static_cast<return_codes>(setjmp(sched.main_context));

                switch (why) {
                case return_codes::saved:
                {
                    impl::cur_thread = &run_queue.front();
                    run_queue.pop_front();
                    runnable--;
                    impl::cur_thread->s = state::running;

                    auto stack_ptr = reinterpret_cast<char*>(impl::cur_thread)
                                     + sizeof(tcb) + sizeof(stack_guard) - impl::cur_thread->stack_sz;
                    auto res = check_stack_smash(stack_ptr, impl::cur_thread->stack_sz);
                    if (!res.result)
                    {
                        tos_debug_print("Init Stack Smash Detected! %llx %llx %llx", res.val1, res.val2, res.val3);
                        while (true);
                    }

                    switch_context(impl::cur_thread->context, return_codes::scheduled);
                }
                break;
                case return_codes::do_exit:
                {
                    auto stack_ptr = reinterpret_cast<char*>(impl::cur_thread)
                            + sizeof(tcb) + sizeof(stack_guard) - impl::cur_thread->stack_sz;
                    std::destroy_at(impl::cur_thread);
                    tos_stack_free(stack_ptr);
                    num_threads--;
                }
                break;
                case return_codes::yield:
                case return_codes::suspend:
                {
                    auto stack_ptr = reinterpret_cast<char*>(impl::cur_thread)
                                     + sizeof(tcb) + sizeof(stack_guard) - impl::cur_thread->stack_sz;
                    auto res = check_stack_smash(stack_ptr, impl::cur_thread->stack_sz);
                    if (!res.result)
                    {
                        tos_debug_print("Stack Smash Detected! %llx %llx %llx", res.val1, res.val2, res.val3);
                        while (true);
                    }
                }
                break;
                default:
                    break;
                }

                impl::cur_thread = nullptr;
                return exit_reason::yield;
            }
        }

        void make_runnable(tcb& t)
        {
            //tos_debug_print("runnable %p\n", &t);

            if (t.s == state::runnable)
            {
                tos_debug_print("already runnable");
                while (true);
            }

            t.s = state::runnable;
            sched.runnable++;

            if (sched.runnable > sched.num_threads)
            {
                tos_debug_print("run queue bug");
                while (true);
            }

            sched.run_queue.push_back(t);
        }
    }

    void this_thread::exit(void*)
    {
        thread_exit();
    }
}


