#pragma once

#include <csetjmp>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <new>
#include <tos/compiler.hpp>
#include <tos/debug/panic.hpp>
#include <tos/ft.hpp>
#include <tos/interrupt.hpp>
#include <tos/scheduler.hpp>
#include <tos/span.hpp>
#include <tos/stack_storage.hpp>
#include <tos/tcb.hpp>

namespace tos {
struct threading_state {
    kern::processor_state backup_state;
    kern::tcb* current_thread;
    int8_t num_threads = 0;
};

namespace global {
extern threading_state thread_state;
}

/**
 * Returns a pointer to the currently running thread.
 *
 * Returns `nullptr` if there's no active thread at the moment.
 *
 * @return pointer to the current thread
 */
ALWAYS_INLINE kern::tcb* self() {
    return global::thread_state.current_thread;
}
}

namespace tos::this_thread {
inline thread_id_t get_id() {
    if (!self())
        return {static_cast<uintptr_t>(-1)};
    return {reinterpret_cast<uintptr_t>(self())};
}
} // namespace tos::this_thread

namespace tos {
context& current_context();

namespace this_thread {
inline void yield() {
    tos::int_guard ig;
    kern::processor_state ctx;
    if (save_context(*self(), ctx) == return_codes::saved) {
        kern::make_runnable(*self());
        switch_context(global::thread_state.backup_state, return_codes::yield);
    }
}

inline void block_forever() {
    kern::disable_interrupts();
    switch_context(global::thread_state.backup_state, return_codes::suspend);
}
} // namespace this_thread

namespace kern {
[[noreturn]] inline void thread_exit() {
    kern::disable_interrupts();

    // no need to save the current context, we'll exit

    switch_context(global::thread_state.backup_state, return_codes::do_exit);
}

inline void suspend_self(const no_interrupts&) {
    kern::processor_state ctx;
    if (save_context(*self(), ctx) == return_codes::saved) {
        switch_context(global::thread_state.backup_state, return_codes::suspend);
    }
}

template<bool FreeStack, class FunT, class... Args>
struct super_tcb final : tcb {
    template<class FunU, class... ArgUs>
    super_tcb(uint16_t stk_sz, FunU&& fun, ArgUs&&... args)
        : tcb(current_context())
        , m_tcb_off(stk_sz - sizeof(super_tcb))
        , m_fun{std::forward<FunU>(fun)}
        , m_args{std::forward<ArgUs>(args)...} {
    }

    // This must not be inlined so that we don't mess up with the compiler's
    // stack allocation assumptions.
    [[noreturn]] NO_INLINE void start() {
        // interrupts should be enabled before entering _user space_
        kern::enable_interrupts();

        std::apply(m_fun, m_args);

        this_thread::exit(nullptr);
    }

    ~super_tcb() final {
        if constexpr (FreeStack) {
            delete[] get_task_base();
        }
    }

private:
    /**
     * This function computes the beginning of the memory block
     * of the task this tcb belongs to.
     *
     * @return pointer to the task's base
     */
    char* get_task_base() {
        return reinterpret_cast<char*>(this) - m_tcb_off;
    }

    uint16_t m_tcb_off; // we store the offset of this object from the task base
    FunT m_fun;
    std::tuple<Args...> m_args;
};

template<bool FreeStack, class FuncT, class... ArgTs>
using lambda_task = super_tcb<FreeStack,
                              std::decay_t<std::remove_reference_t<FuncT>>,
                              std::decay_t<std::remove_reference_t<ArgTs>>...>;

template<bool FreeStack, class FuncT, class... ArgTs>
lambda_task<FreeStack, FuncT, ArgTs...>&
prep_lambda_layout(tos::span<uint8_t> task_data, FuncT&& func, ArgTs&&... args) {
    // the tcb lives at the top of the stack
    const auto stack_top = task_data.end();

    static_assert(std::is_invocable_v<FuncT, ArgTs...>,
                  "Arguments passed to tos::launch do not match the function!");

    const auto t_ptr = stack_top - sizeof(lambda_task<FreeStack, FuncT, ArgTs...>);

    auto thread = new (t_ptr) lambda_task<FreeStack, FuncT, ArgTs...>(
        task_data.size(), std::forward<FuncT>(func), std::forward<ArgTs>(args)...);

    return *thread;
}

template<class TaskT>
TOS_SIZE_OPTIMIZE thread_id_t start(TaskT& t) {
    static_assert(std::is_base_of<tcb, TaskT>{}, "Tasks must inherit from tcb class!");

    // New threads are runnable by default.
    make_runnable(t);
    global::thread_state.num_threads++;

    // prepare the initial ctx for the new task
    auto ctx_ptr = new ((char*)&t - sizeof(processor_state)) processor_state;

    kern::disable_interrupts();
    if (save_context(t, *ctx_ptr) == return_codes::saved) {
        kern::enable_interrupts();
        return {reinterpret_cast<uintptr_t>(static_cast<tcb*>(&t))};
    }

    // TODO(#35): If a non maskable interrupt fires here, we're toast.

    /**
     * this is the actual entry point of the thread.
     * will be called when scheduled
     *
     * set the stack pointer so the new thread will have an
     * independent execution context
     */
    arch::set_stack_ptr(reinterpret_cast<char*>(self()));

    static_cast<TaskT*>(self())->start();

    TOS_UNREACHABLE();
}
} // namespace kern

template<bool FreeStack, class FuncT, class... ArgTs>
auto& launch(tos::span<uint8_t> task_span, FuncT&& func, ArgTs&&... args) {
    auto& t = kern::prep_lambda_layout<FreeStack>(
        task_span, std::forward<FuncT>(func), std::forward<ArgTs>(args)...);
    start(t);
    return t;
}

template<class FuncT, class... ArgTs>
auto& launch(stack_size_t stack_sz, FuncT&& func, ArgTs&&... args) {
    auto ptr = new (std::nothrow) char[stack_sz.sz];
    if (!ptr) {
        tos::debug::panic("Stack allocation failed");
    }
    tos::span<uint8_t> task_span(reinterpret_cast<uint8_t*>(ptr), stack_sz.sz);
    auto& res =
        launch<true>(task_span, std::forward<FuncT>(func), std::forward<ArgTs>(args)...);
    return res;
}

template<class FuncT, class... ArgTs, size_t StSz>
auto& launch(stack_storage<StSz>& stack, FuncT&& func, ArgTs&&... args) {
    return launch<false>(stack, std::forward<FuncT>(func), std::forward<ArgTs>(args)...);
}

inline void this_thread::exit(void*) {
    kern::thread_exit();
}
} // namespace tos
