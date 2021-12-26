#pragma once

#include <memory>
#include <tos/concepts.hpp>
#include <tos/function_ref.hpp>
#include <tos/processor_context.hpp>

namespace tos {
struct any_fiber {
    virtual void suspend() = 0;
    virtual void resume() = 0;
    virtual void destroy() = 0;
    virtual ~any_fiber() = default;
};

template<class T>
struct basic_fiber;

template<class T>
concept Fiber = DerivedFrom<T, any_fiber>;

template<class CrtpT>
struct basic_fiber : any_fiber {
    void resume() override;

    using suspend_t = void;
    template<class FnT>
    suspend_t suspend(FnT&&);

    suspend_t suspend() override {
        return suspend([] {});
    }

    void destroy() override {}

    [[noreturn]] void suspend_final(context_codes code = context_codes::suspend);

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

    void run_on_resume() {
        if constexpr(requires (CrtpT* t) { t->on_resume(); }) {
            this->self()->on_resume();
        }
    }

    void run_on_suspend() {
        if constexpr(requires (CrtpT* t) { t->on_suspend(); }) {
            this->self()->on_suspend();
        }
    }

    void run_on_start() {
        if constexpr(requires (CrtpT* t) { t->on_start(); }) {
            this->self()->on_start();
        }
    }
    
private:
    CrtpT* self() {
        return static_cast<CrtpT*>(this);
    }

    // If the fiber is executing, this stores the context of our resumer.
    // If the fiber is suspended, this stores the context of us.
    processor_context* m_ctx;
};

template<Fiber FibT>
function_ref<void()> fiber_resumer(FibT& fib) {
    return tos::mem_function_ref<&any_fiber::resume>(fib);
}

template<class T>
void basic_fiber<T>::resume() {
    processor_context caller_ctx;
    auto fiber_ctx = std::exchange(m_ctx, &caller_ctx);
    auto save_res = save_ctx(caller_ctx);

    switch (save_res) {
    case context_codes::saved:
        switch_context(*fiber_ctx, context_codes::scheduled);
        TOS_UNREACHABLE();
    case context_codes::suspend:
        break;
    case context_codes::do_exit:
        this->destroy();
        break;
    case context_codes::scheduled:
        TOS_UNREACHABLE();
    }
}

#define save_context(fib, ctx) (fib).set_processor_state((ctx)), save_ctx(ctx)
template<class T>
template<class FnT>
auto basic_fiber<T>::suspend(FnT&& before_switch) -> suspend_t {
    auto caller_ctx_ptr = m_ctx;
    processor_context ctx;

    if (save_context(*this, ctx) == context_codes::saved) {
        before_switch();
        run_on_suspend();
        switch_context(*caller_ctx_ptr, context_codes::suspend);
    }

    run_on_resume();
}

template<class T>
[[noreturn]] inline void basic_fiber<T>::suspend_final(context_codes code) {
    run_on_suspend();
    switch_context(*m_ctx, code);
}

// Transfers control symmetrically to the resume fiber from the currently
// executing suspend fiber.
// Behaviour is undefined if suspend is not currently executing.
// Upon resume suspending, the caller of suspend will be resumed.
// In other words:
//
//      a: b.resume():
//      b: swap_fibers(b, c);
//      c: c.suspend();
//      a is resumed
void swap_fibers(Fiber auto& suspend, Fiber auto& resume) {
    processor_context context;

    auto caller_ctx_ptr = &suspend.get_processor_state();
    auto resume_ctx_ptr = &resume.get_processor_state();

    resume.set_processor_state(*caller_ctx_ptr);
    suspend.set_processor_state(context);

    suspend.run_on_suspend();
    swap_context(context, *resume_ctx_ptr);

    suspend.run_on_resume();
}
} // namespace tos