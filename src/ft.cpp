#include <setjmp.h>
#include <stdlib.h>
#include "tos/ft.hpp"
#include "tos/thread_info.hpp"
#include "tos/scheduler.hpp"

extern "C"
{
void tos_set_stack_ptr(void *ptr);
void tos_power_down();
void* tos_stack_alloc(size_t size);
void tos_stack_free(void*);
void tos_shutdown();
}

enum return_codes {
    none = 0,
    yield,
    wait,
    do_exit
};

namespace ft {
    intrusive_list<thread_info> run_queue;

    thread_info *cur_thread = nullptr;
    jmp_buf global_one;

    namespace this_thread {
        void yield() {
            if (setjmp(cur_thread->buffer) == 0) {
                // yielded
                make_runnable(cur_thread);
                longjmp(global_one, return_codes::yield);
            } else {
                // We're back to this thread, upon returning from
                // yield, control will be back to the thread
            }
        }

        int get_id() {
            if (!cur_thread) return -1;
            return cur_thread->id;
        }
    }

    void wait_yield() {
        if (setjmp(cur_thread->buffer) == 0) {
            // yielded
            longjmp(global_one, return_codes::wait);
        } else {
            // We're back to this thread, upon returning from
            // yield, control will be back to the thread
        }
    }

    void thread_exit() {
        longjmp(global_one, return_codes::do_exit);
    }

    thread_info *self() {
        return cur_thread;
    }

    thread_info **threads = nullptr;
    int num_threads = 0;
    int capacity = 0;

    void start(void (*t_start)()) {
        int index;
        if (capacity == num_threads) {
            auto new_one = new thread_info *[capacity + 1];
            for (int i = 0; i < num_threads; ++i) {
                new_one[i] = threads[i];
            }
            delete threads;
            threads = new_one;
            capacity++;
            index = num_threads;
        } else {
            for (auto t = threads; t != threads + capacity; ++t) {
                if (t == nullptr) {
                    index = t - threads;
                    break;
                }
            }
        }

        constexpr auto stack_size = 256;

        auto thread = new thread_info();
        num_threads++;
        thread->stack = tos_stack_alloc(stack_size);
        thread->entry = t_start;
        thread->id = num_threads;
        threads[index] = thread;

        run_queue.push_back(*thread);

        if (setjmp(thread->buffer) == 0) {
            //done
            return;
        }

        // this is the actual entry point of the thread
        // will be called when scheduled

        tos_set_stack_ptr((void *) ((uintptr_t) cur_thread->stack + stack_size));

        cur_thread->entry();
        this_thread::exit(nullptr);
    }

    void this_thread::exit(void *) {
        thread_exit();
    }

    void schedule() {
        if (num_threads == 0)
        {
            // no thread left
            tos_shutdown();
        }

        if (run_queue.empty()) {
            return tos_power_down();
        }

        auto why = static_cast<return_codes>(setjmp(global_one));
        switch (why) {
            case none:
                // run it
                cur_thread = &run_queue.front();
                run_queue.pop_front();
                longjmp(cur_thread->buffer, 1);
                break;
            case yield:
                cur_thread = nullptr;
                break;
            case wait:
                // nothing
                break;
            case do_exit:
                tos_stack_free(cur_thread->stack);
                cur_thread->stack = nullptr;
                delete cur_thread;
                num_threads--;
                break;
        }
    }

    void make_runnable(thread_info *t) {
        run_queue.push_back(*t);
    }
}


