#pragma once

#include "tos/compiler.hpp"
#include "tos/debug/log.hpp"
#include "tos/detail/coro.hpp"
#include <tos/core/arch.hpp>
#include <tos/processor_context.hpp>
#include <tos/span.hpp>
#include <utility>

namespace tos {
struct basic_fiber {
    template<class FnT>
    void resume(FnT&&);

    void resume() {
        resume([] {});
    }

    using suspend_t = void;
    template<class FnT>
    suspend_t suspend(FnT&&);

    suspend_t suspend() {
        return suspend([] {});
    }

    virtual ~basic_fiber() = default;
    virtual void start() = 0;

    /**
     * Returns a reference to the context of the task.
     *
     * The function can either be called to store the current
     * context, or to resume execution from the context.
     *
     * @return execution context of the task
     */
    processor_context& get_processor_state() {
        return *m_ctx;
    }

    void set_processor_state(processor_context& buf) {
        m_ctx = &buf;
    }

private:
    // If the fiber is executing, this stores the context of our resumer.
    // If the fiber is suspended, this stores the context of us.
    processor_context* m_ctx;

    template<class StartFn>
    friend basic_fiber& start(span<uint8_t> stack, StartFn&& fn);
};

namespace fiber {
template<class StartFn>
auto& start(span<uint8_t> stack, StartFn&& fn) {
    struct fib : basic_fiber {
        NO_INLINE
        void start() final {
            m_fn(*this);
        }

        fib(StartFn&& fn, span<uint8_t> stack)
            : m_fn{std::forward<StartFn>(fn)}
            , m_full_stack(stack) {
        }

        StartFn m_fn;
        span<uint8_t> m_full_stack;
    };

    tos::debug::log("Stack top at", stack.end());

    auto res = new (stack.end() - sizeof(fib)) fib{std::forward<StartFn>(fn), stack};
    stack = stack.slice(0, stack.size() - sizeof(fib));
    tos::debug::log("Stack top at", stack.end());

    auto ctx_ptr = new (stack.end() - sizeof(processor_context)) processor_context;
    stack = stack.slice(0, stack.size() - sizeof(processor_context));
    tos::debug::log("Stack top at", stack.end());

    auto setup_fiber_init_stack = [](span<uint8_t> stack, basic_fiber* fib_ptr) {
        auto ptr_pos = stack.end() - sizeof fib_ptr;
        memcpy(ptr_pos, &fib_ptr, sizeof fib_ptr);
        ptr_pos = stack.end() - 2* sizeof fib_ptr;
        memcpy(ptr_pos, &fib_ptr, sizeof fib_ptr);
        ptr_pos = stack.end() - 3* sizeof fib_ptr;
        memcpy(ptr_pos, &fib_ptr, sizeof fib_ptr);
        ptr_pos = stack.end() - 4* sizeof fib_ptr;
        memcpy(ptr_pos, &fib_ptr, sizeof fib_ptr);
        ptr_pos = stack.end() - 5* sizeof fib_ptr;
        memcpy(ptr_pos, &fib_ptr, sizeof fib_ptr);

        return std::pair(
            +[]() {
                fib* cur_fib;
                auto sp = cur_arch::get_stack_ptr();
                memcpy(&cur_fib, sp, sizeof cur_fib);
                // cur_arch::set_stack_ptr(static_cast<char*>(sp) + sizeof cur_fib);
                cur_fib->start();
                TOS_UNREACHABLE();
            },
            stack.end() - sizeof fib_ptr);
    };

    auto [entry_point, sp] = setup_fiber_init_stack(stack, res);

    start(*ctx_ptr, entry_point, sp);
    res->set_processor_state(*ctx_ptr);

    return *res;
}
} // namespace fiber
} // namespace tos

namespace tos {
template<class FnT>
inline void basic_fiber::resume(FnT&& before_switch) {
    processor_context caller_ctx;
    auto fiber_ctx = std::exchange(m_ctx, &caller_ctx);
    auto save_res = save_ctx(caller_ctx);

    switch (save_res) {
    case context_codes::saved:
        before_switch();
        switch_context(*fiber_ctx, context_codes::scheduled);
        TOS_UNREACHABLE();
    case context_codes::yield:
    case context_codes::suspend:
        break;
    case context_codes::do_exit:
        std::destroy_at(this);
        break;
    case context_codes::scheduled:
        TOS_UNREACHABLE();
    }
}
#define save_fib_context(fib, ctx) (fib).set_processor_state((ctx)), save_ctx(ctx)
template<class FnT>
inline auto basic_fiber::suspend(FnT&& before_switch) -> suspend_t {
    auto caller_ctx_ptr = m_ctx;
    processor_context ctx;

    if (save_fib_context(*this, ctx) == context_codes::saved) {
        before_switch();
        switch_context(*caller_ctx_ptr, context_codes::suspend);
    }
}
} // namespace tos
#undef save_fib_context
