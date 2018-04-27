#include <setjmp.h>
#include <stdlib.h>
#include "tos/ft.hpp"
#include "tos/thread_info.hpp"
#include "tos/scheduler.hpp"

#include <tos/print.hpp>
#include <tos_arch.hpp>
#include <tos/arch.hpp>

namespace tos {
    namespace {
        struct scheduler
        {
            thread_info** threads = nullptr;
            int num_threads = 0;
            int capacity = 0;
            jmp_buf main_context{};

            void start(void (* t_start)());

            void schedule();
        };
    }
}

namespace tos {
    enum class return_codes : uint8_t
    {
        saved = 0,
        /**
         * a running thread yielded
         */
        yield,
        /**
         * a running thread is waiting on something
         */
        do_wait,
        /**
         * a thread exited
         */
        do_exit,
        /**
         * the thread was assigned the cpu
         */
        scheduled
    };

    [[noreturn]] static void switch_context(jmp_buf& j, return_codes rc)
    {
        longjmp(j, static_cast<int>(rc));
    }
}

namespace tos {
    scheduler sc;
    intrusive_list<thread_info> run_queue;

    thread_info* cur_thread = nullptr;

    namespace this_thread {
        void yield()
        {
            tos::disable_interrupts();
            if (setjmp(cur_thread->context)==(int) return_codes::saved) {
                // yielded
                make_runnable(cur_thread);
                switch_context(sc.main_context, return_codes::yield);
            }
            else {
                tos::enable_interrupts();
                // We're back to this thread, upon returning from
                // yield, control will be back to the thread
            }
        }

        int get_id()
        {
            if (!cur_thread) return -1;
            return cur_thread->id;
        }
    }

    void wait_yield()
    {
        //tos::avr::write_sync("wait\n", 6);
        tos::disable_interrupts();
        if (setjmp(cur_thread->context)==(int) return_codes::saved) {
            // yielded
            switch_context(sc.main_context, return_codes::do_wait);
        }
        else {
            tos::enable_interrupts();
            // We're back to this thread, upon returning from
            // yield, control will be back to the thread
        }
    }

    static void thread_exit()
    {
        tos::disable_interrupts();
        // no need to save the current context
        switch_context(sc.main_context, return_codes::do_exit);
    }

    thread_info* self()
    {
        return cur_thread;
    }

    void launch(entry_t e)
    {
        sc.start(e);
    }

    void schedule()
    {
        sc.schedule();
    }

    void scheduler::start(void (* t_start)())
    {
        int index = 0;
        if (capacity==num_threads) {
            auto new_one = new thread_info* [capacity+1];
            for (int i = 0; i<num_threads; ++i) {
                new_one[i] = threads[i];
            }
            delete threads;
            threads = new_one;
            capacity++;
            index = num_threads;
        }
        else {
            for (auto t = threads; t!=threads+capacity; ++t) {
                if (t==nullptr) {
                    index = t-threads;
                    break;
                }
            }
        }

        constexpr auto stack_size = 1024;

        auto thread = new thread_info();
        num_threads++;
        thread->stack = tos_stack_alloc(stack_size);
        thread->entry = t_start;
        thread->id = num_threads;
        threads[index] = thread;

        run_queue.push_back(*thread);

        tos::disable_interrupts();

        /*
         * save the current processor state as the
         * threads current state
         * we'll pick it up from here when it's
         * scheduled
         */
        if (setjmp(thread->context)==(int) return_codes::saved) {
            //done
            tos::enable_interrupts();
            return;
        }

        // this is the actual entry point of the thread.
        // will be called when scheduled

        /*
         * set the stack pointer so the new thread will have an
         * independent execution context
         *
         * not sure if doing this with such a function call will
         * be portable in all ABIs
         */
        tos_set_stack_ptr((void*) ((uintptr_t) cur_thread->stack+stack_size));

        tos::enable_interrupts();

        cur_thread->entry();
        this_thread::exit(nullptr);
    }

    void this_thread::exit(void*)
    {
        thread_exit();
    }

    uint8_t runnable_count()
    {
        return run_queue.size();
    }

    void scheduler::schedule()
    {
        if (num_threads==0) {
            // no thread left, potentially a bug
            //tos::avr::write_sync("reboot\n", 8);
            tos_shutdown();
        }

        if (run_queue.empty()) {
            /*
             * there's no thread to run right now
             */
            return tos_power_down();
        }

        // interrupts are disabled during a context switch
        tos::disable_interrupts();
        auto why = static_cast<return_codes>(setjmp(sc.main_context));

        switch (why) {
        case return_codes::saved:
            // run it
            cur_thread = &run_queue.front();
            run_queue.pop_front();
            // thread should enable interrupts
            switch_context(cur_thread->context, return_codes::scheduled);
        case return_codes::yield:
            cur_thread = nullptr;
            tos::enable_interrupts();
            break;
        case return_codes::do_wait:
            // nothing
            tos::enable_interrupts();
            break;
        case return_codes::do_exit:
            tos_stack_free(cur_thread->stack);
            delete cur_thread;
            num_threads--;
            cur_thread = nullptr;
            tos::enable_interrupts();
            break;
        case return_codes::scheduled:
            break;
        }
    }

    void make_runnable(thread_info* t)
    {
        run_queue.push_back(*t);
    }
}


