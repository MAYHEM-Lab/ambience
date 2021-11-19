#pragma once

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