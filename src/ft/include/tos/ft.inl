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
#include <type_traits>
#include <tos/threading_state.hpp>

namespace tos::this_thread {
inline thread_id_t get_id() {
    if (!self())
        return {static_cast<uintptr_t>(-1)};
    return {reinterpret_cast<uintptr_t>(self())};
}
} // namespace tos::this_thread

namespace tos {
context& current_context();

namespace kern {
[[noreturn]] void thread_exit();
void suspend_self(const no_interrupts&);

template<class Deleter, class Starter>
struct mixin_tcb
    : tcb
    , Starter
    , Deleter {

    using deleter_type = Deleter;
    using starter_type = Starter;

    void destroy() override {
        Deleter::destroy();
    }

    template<class... StarterArg>
    explicit mixin_tcb(uint8_t* stack_base, StarterArg&&... starter)
        : tcb(current_context())
        , Starter{std::forward<StarterArg>(starter)...}
        , Deleter(stack_base) {
    }
};

struct deleter {
    explicit deleter(uint8_t* stack_base)
        : m_stack_base{stack_base} {
    }

    void destroy();

    char* get_task_base() const {
        return reinterpret_cast<char*>(m_stack_base);
    }

    uint8_t* m_stack_base;
};

struct no_delete {
    void destroy() {};
    explicit no_delete(uint8_t*) {
    }
};

template<class FuncT, class... ArgTs>
struct apply_starter {
    template<class FuncU, class... ArgUs>
    explicit apply_starter(FuncU&& func, ArgUs&&... args)
        : m_fun{std::forward<FuncU>(func)}
        , m_args{std::forward<ArgUs>(args)...} {
    }

    static_assert(std::is_invocable_v<FuncT, ArgTs...>,
                  "Arguments passed to tos::launch do not match the function!");

    [[noreturn]] void start() {
        // interrupts should be enabled before entering _user space_
        kern::enable_interrupts();

        std::apply(m_fun, m_args);

        this_thread::exit(nullptr);
    }

    FuncT m_fun;
    std::tuple<ArgTs...> m_args;
};

template<class FuncT, class... ArgTs>
using apply_starter_t = apply_starter<std::decay_t<std::remove_reference_t<FuncT>>,
                                      std::decay_t<std::remove_reference_t<ArgTs>>...>;

template<class TaskType, class... ArgTs>
TaskType& prepare_task_layout(tos::span<uint8_t> task_data, ArgTs&&... args) {
    // the tcb lives at the top of the stack
    const auto stack_top = task_data.end();

    const auto t_ptr = stack_top - sizeof(TaskType);

    auto thread = new (t_ptr) TaskType(task_data.data(), std::forward<ArgTs>(args)...);

    return *thread;
}

template<class TaskT>
#if defined(__x86_64__)
[[gnu::force_align_arg_pointer]]
#endif
void start_cur() {
    static_cast<TaskT*>(self())->start();
}

thread_id_t start(tcb& t, void (*entry)());
} // namespace kern

template<class TaskType, class FuncT, class... ArgTs>
auto& raw_launch(tos::span<uint8_t> task_span, FuncT&& func, ArgTs&&... args) {
    auto& t = kern::prepare_task_layout<TaskType>(
        task_span, std::forward<FuncT>(func), std::forward<ArgTs>(args)...);
    start(t, &kern::start_cur<std::remove_reference_t<decltype(t)>>);
    return t;
}

template<class FuncT, class... ArgTs>
auto& launch(tos::span<uint8_t> task_span, FuncT&& func, ArgTs&&... args) {
    using lambda_task =
        kern::mixin_tcb<kern::no_delete, kern::apply_starter_t<FuncT, ArgTs...>>;

    return raw_launch<lambda_task>(
        task_span, std::forward<FuncT>(func), std::forward<ArgTs>(args)...);
}

template<class FuncT, class... ArgTs>
auto& launch(stack_size_t stack_sz, FuncT&& func, ArgTs&&... args) {
    auto ptr = new (std::nothrow) char[stack_sz.sz];
    if (!ptr) {
        tos::debug::panic("Stack allocation failed");
    }
    tos::span<uint8_t> task_span(reinterpret_cast<uint8_t*>(ptr), stack_sz.sz);

    using lambda_task =
        kern::mixin_tcb<kern::deleter, kern::apply_starter_t<FuncT, ArgTs...>>;

    auto& res = raw_launch<lambda_task>(
        task_span, std::forward<FuncT>(func), std::forward<ArgTs>(args)...);
    return res;
}

template<class FuncT, class... ArgTs, size_t StSz>
auto& launch(stack_storage<StSz>& stack, FuncT&& func, ArgTs&&... args) {
    using lambda_task =
        kern::mixin_tcb<kern::no_delete, kern::apply_starter_t<FuncT, ArgTs...>>;

    return raw_launch<lambda_task>(
        stack, std::forward<FuncT>(func), std::forward<ArgTs>(args)...);
}
} // namespace tos
