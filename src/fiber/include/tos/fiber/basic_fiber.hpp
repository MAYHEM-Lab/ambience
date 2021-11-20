#pragma once

#include <memory>
#include <tos/processor_context.hpp>

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

    template<class FnT>
    [[noreturn]] void suspend_final(FnT&& before_switch);

    virtual ~basic_fiber() = default;
    [[noreturn]] virtual void start() = 0;

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
};

template<class FnT>
void basic_fiber::resume(FnT&& before_switch) {
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
auto basic_fiber::suspend(FnT&& before_switch) -> suspend_t {
    auto caller_ctx_ptr = m_ctx;
    processor_context ctx;

    if (save_fib_context(*this, ctx) == context_codes::saved) {
        before_switch();
        switch_context(*caller_ctx_ptr, context_codes::suspend);
    }
}
template<class FnT>
[[noreturn]] void basic_fiber::suspend_final(FnT&& before_switch) {
    switch_context(*m_ctx, context_codes::suspend);
}

// Transfers control symmetrically to the resume fiber from the currently
// executing suspend fiber.
// Behaviour is undefined if basic_fiber is not currently executing.
// Upon resume suspending, the caller of suspend will be resumed.
// In other words:
//      
//      a: b.resume():
//      b: swap_fibers(b, c);
//      c: c.suspend();
//      a is resumed
template <class FnT>
void swap_fibers(basic_fiber& suspend, basic_fiber& resume, FnT&& before_switch) {
    processor_context context;

    auto caller_ctx_ptr = &suspend.get_processor_state();
    auto resume_ctx_ptr = &resume.get_processor_state();

    resume.set_processor_state(*caller_ctx_ptr);
    suspend.set_processor_state(context);

    before_switch();
    swap_context(context, *resume_ctx_ptr);
}
} // namespace tos
#undef save_fib_context