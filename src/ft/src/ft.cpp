#include <setjmp.h>
#include <stdlib.h>
#include <tos/ft.hpp>
#include <tos/thread_info.hpp>
#include <tos/scheduler.hpp>

#include <tos_arch.hpp>
#include <tos/interrupt.hpp>
#include <tos/new.hpp>
#include <tos/memory.hpp>
#include <nrf_delay.h>

namespace tos {
    namespace {
        struct scheduler
        {
            jmp_buf main_context{};
            int8_t  num_threads = 0;
            uint8_t busy = 0;
            intrusive_list<thread_info> run_queue;

            void start(void (* t_start)());

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

void ledOn(uint32_t pin);
void ledOff(uint32_t pin);

void led1_task();
namespace tos {
    scheduler sc;
    namespace impl
    {
        thread_info* cur_thread = nullptr;
    }

    namespace this_thread
    {
        void yield()
        {
            tos::int_guard ig;
            if (setjmp(impl::cur_thread->context)==(int) return_codes::saved) {
                make_runnable(impl::cur_thread);
                switch_context(sc.main_context, return_codes::yield);
            }
        }
    }

    void wait_yield()
    {
        if (setjmp(impl::cur_thread->context)==(int) return_codes::saved) {
            switch_context(sc.main_context, return_codes::do_wait);
        }
    }

    static void thread_exit()
    {
        tos::disable_interrupts();

        // no need to save the current context, we'll exit

        switch_context(sc.main_context, return_codes::do_exit);
    }

    void launch(thread_info::entry_point_t e)
    {
        sc.start(e);
    }

    exit_reason schedule()
    {
        return sc.schedule();
    }

    constexpr auto stack_size = 512;
    void scheduler::start(void (* t_start)())
    {
        const auto stack = static_cast<char*>(tos_stack_alloc(stack_size));
        const auto t_ptr = stack + stack_size - sizeof(thread_info);

        auto thread = new (t_ptr) thread_info(t_start);

        // New threads are runnable by default.
        run_queue.push_back(*thread);
        num_threads++;

        tos::disable_interrupts();
        if (setjmp(thread->context)==(int) return_codes::saved) {
            tos::enable_interrupts();
            return;
        }

        /**
         * this is the actual entry point of the thread.
         * will be called when scheduled
         *
         * set the stack pointer so the new thread will have an
         * independent execution context
         */
        auto stck = reinterpret_cast<uintptr_t>(impl::cur_thread);

        if ((stck % 8) == 0)
        {
            ledOn(19);
            nrf_delay_ms(1000);
            ledOff(19);
            nrf_delay_ms(1000);
        }

        tos_set_stack_ptr((char*)stck);

        ledOn(19);
        nrf_delay_ms(1000);
        ledOff(19);
        nrf_delay_ms(1000);

        tos::enable_interrupts();
        ledOn(19);
        nrf_delay_ms(1000);
        ledOff(19);
        nrf_delay_ms(1000);

        impl::cur_thread->entry();

        ledOn(19);
        nrf_delay_ms(1000);
        ledOff(19);
        nrf_delay_ms(1000);
        this_thread::exit(nullptr);
    }

    void this_thread::exit(void*)
    {
        thread_exit();
    }

    void busy() { sc.busy++; }
    void unbusy() { sc.busy--; }

    exit_reason scheduler::schedule()
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

                if (sc.busy > 0)
                {
                    return exit_reason::idle;
                }

                return exit_reason::power_down;
            }

            auto why = static_cast<return_codes>(setjmp(sc.main_context));

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
                        + sizeof(thread_info) - stack_size;
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

    void make_runnable(thread_info* t)
    {
        sc.run_queue.push_back(*t);
    }
}


