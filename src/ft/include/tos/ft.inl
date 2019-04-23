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
#include <tos/stack_storage.hpp>

namespace tos {
namespace this_thread {
inline thread_id_t get_id() {
    if (!impl::cur_thread) return {static_cast<uintptr_t>(-1)};
    return {reinterpret_cast<uintptr_t>(impl::cur_thread)};
}
}
}

namespace tos {
extern kern::scheduler sched;

namespace this_thread {
inline void yield() {
    tos::int_guard ig;
    kern::ctx ctx;
    if (save_context(*impl::cur_thread, ctx) == return_codes::saved) {
        kern::make_runnable(*impl::cur_thread);
        switch_context(sched.main_context, return_codes::yield);
    }
}

inline void block_forever() {
    kern::disable_interrupts();
    switch_context(sched.main_context, return_codes::suspend);
}
}

namespace kern {
[[noreturn]]
inline void thread_exit() {
    kern::disable_interrupts();

    // no need to save the current context, we'll exit

    switch_context(sched.main_context, return_codes::do_exit);
}

inline void suspend_self(const int_guard&) {
    // interrupts are assumed to be disabled for this function to be called
    //tos_debug_print("suspend %p\n", impl::cur_thread);

    kern::ctx ctx;
    if (save_context(*impl::cur_thread, ctx) == return_codes::saved) {
        switch_context(sched.main_context, return_codes::suspend);
    }
}

inline exit_reason schedule() {
    return sched.schedule();
}

template<bool FreeStack, class FunT, class... Args>
struct super_tcb : tcb {
    template<class FunU, class... ArgUs>
    super_tcb(uint16_t stk_sz, FunU &&fun, ArgUs &&... args)
        : m_tcb_off(stk_sz - sizeof(super_tcb)),
          m_fun{std::forward<FunU>(fun)},
          m_args{std::forward<ArgUs>(args)...} {}

    // This must not be inlined so that we don't mess up with the compiler's
    // stack allocation assumptions.
    void NORETURN NO_INLINE start() {
        // interrupts should be enabled before entering _user space_
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
    char *get_task_base() {
        return reinterpret_cast<char *>(this) - m_tcb_off;
    }

    ~super_tcb() final {
        if (FreeStack) tos_stack_free(get_task_base());
    }

private:
    uint16_t m_tcb_off; // we store the offset of this object from the task base
    FunT m_fun;
    std::tuple<Args...> m_args;
};

template<bool FreeStack, class FuncT, class... ArgTs>
using lambda_task = super_tcb<FreeStack, std::decay_t<std::remove_reference_t<FuncT>>, std::decay_t<std::remove_reference_t<ArgTs>>...>;

template<bool FreeStack, class FuncT, class... ArgTs>
lambda_task<FreeStack, FuncT, ArgTs...> &
prep_lambda_layout(tos::span<char> task_data, FuncT &&func, ArgTs &&... args) {
    // the tcb lives at the top of the stack
    const auto stack_top = task_data.end();

    const auto t_ptr = stack_top - sizeof(lambda_task<FreeStack, FuncT, ArgTs...>);

    auto thread = new(t_ptr) lambda_task<FreeStack, FuncT, ArgTs...>(
        task_data.size(),
        std::forward<FuncT>(func),
        std::forward<ArgTs>(args)...);

    return *thread;
}

template<class TaskT>
inline thread_id_t scheduler::start(TaskT &t) {
    static_assert(std::is_base_of<tcb, TaskT>{}, "Tasks must inherit from tcb class!");

    // New threads are runnable by default.
    run_queue.push_back(t);
    num_threads++;

    // prepare the initial ctx for the new task
    auto ctx_ptr = new((char *) &t - sizeof(ctx)) ctx;

    kern::disable_interrupts();
    if (save_context(t, *ctx_ptr) == return_codes::saved) {
        kern::enable_interrupts();
        return {reinterpret_cast<uintptr_t>(static_cast<tcb *>(&t))};
    }

    // If a non maskable interrupt fires here, we're probably toast

    /**
     * this is the actual entry point of the thread.
     * will be called when scheduled
     *
     * set the stack pointer so the new thread will have an
     * independent execution context
     */
    tos_set_stack_ptr(reinterpret_cast<char *>(impl::cur_thread));

    static_cast<TaskT *>(impl::cur_thread)->start();

    __builtin_unreachable();
}

inline void busy() {
    sched.busy++;
}

inline void unbusy() {
    sched.busy--;
}

inline exit_reason scheduler::schedule() {
    while (true) {
        if (num_threads == 0) {
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

            if (sched.busy > 0) {
                return exit_reason::idle;
            }

            return exit_reason::power_down;
        }

        auto why = save_ctx(sched.main_context);

        switch (why) {
            case return_codes::saved: {
                impl::cur_thread = &run_queue.front();
                run_queue.pop_front();

                switch_context(impl::cur_thread->get_ctx(), return_codes::scheduled);
            }
            case return_codes::do_exit: {
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

inline void make_runnable(tcb &t) {
    sched.run_queue.push_back(t);
}
}

template<bool FreeStack, class FuncT, class... ArgTs>
inline auto &launch(tos::span<char> task_span, FuncT &&func, ArgTs &&... args) {
    auto &t = kern::prep_lambda_layout<FreeStack>(task_span, std::forward<FuncT>(func), std::forward<ArgTs>(args)...);
    sched.start(t);
    return t;
}

template<class FuncT, class... ArgTs>
inline auto &launch(stack_size_t stack_sz, FuncT &&func, ArgTs &&... args) {
    tos::span<char> task_span((char *) tos_stack_alloc(stack_sz.sz), stack_sz.sz);
    auto &res = launch<true>(task_span, std::forward<FuncT>(func), std::forward<ArgTs>(args)...);
    return res;
}

template<class FuncT, class... ArgTs, size_t StSz>
inline auto &launch(stack_storage<StSz> &stack, FuncT &&func, ArgTs &&... args) {
    tos::span<char> task_span((char *) &stack, StSz);
    return launch<false>(task_span, std::forward<FuncT>(func), std::forward<ArgTs>(args)...);
}

constexpr struct alloc_stack_t {
} alloc_stack;

template<class FuncT, class... ArgTs>
inline auto &launch(alloc_stack_t, FuncT &&func, ArgTs &&... args) {
    constexpr stack_size_t stack_size{TOS_DEFAULT_STACK_SIZE};
    return launch(stack_size, std::forward<FuncT>(func), std::forward<ArgTs>(args)...);
}

inline void this_thread::exit(void *) {
    kern::thread_exit();
}
}


