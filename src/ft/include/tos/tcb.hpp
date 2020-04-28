#pragma once

#include <algorithm>
#include <csetjmp>
#include <cstddef>
#include <tos/arch.hpp>
#include <tos/context.hpp>
#include <tos/intrusive_list.hpp>
#include <tos/utility.hpp>
#include <utility>
#include <tos/components/threads.hpp>

namespace tos::kern {
struct processor_state;

/**
 * This type represents an execution context in the system.
 *
 * It's expected that a concrete task descriptor type will
 * extend this class to implement required functionality such
 * as starting threads or passing arguments.
 */
struct alignas(alignof(std::max_align_t)) tcb : public list_node<tcb> {
    explicit tcb(context& ctx_ptr)
        : m_context{&ctx_ptr} {
        if (auto threads = m_context->get_component<threads_component>(); threads) {
            threads->thread_created(*this);
        }
    }
    /**
     * Returns a reference to the context of the task.
     *
     * The function can either be called to store the current
     * context, or to resume execution from the context.
     *
     * @return execution context of the task
     */
    processor_state& get_processor_state() {
        return *m_ctx;
    }

    void set_processor_state(processor_state& buf) {
        m_ctx = &buf;
    }

    /**
     * The threading subsystem does not know about the concrete time
     * at the destruction of a task, so the destructor must be virtual
     * in order to properly destroy the concrete task descriptor.
     */
    virtual ~tcb() = 0;

    context* m_context;

private:
    processor_state* m_ctx;
};

inline tcb::~tcb() {
    if (auto threads = m_context->get_component<threads_component>(); threads) {
        threads->thread_exited(*this);
    }
}
} // namespace tos::kern

namespace tos {
enum class return_codes : uint8_t
{
    saved = 0,
    /**
     * the running thread yielded
     */
    yield,
    /**
     * the running thread has been suspended
     */
    suspend,
    /**
     * a thread exited
     */
    do_exit,
    /**
     * this thread was assigned the cpu
     */
    scheduled
};

namespace kern {
struct processor_state {
    jmp_buf buf;
};

[[noreturn]] inline void switch_context(kern::processor_state& j, return_codes rc) {
    longjmp(j.buf, static_cast<int>(rc));

    __builtin_unreachable();
}
} // namespace kern
} // namespace tos

#define save_ctx(ctx) (::tos::return_codes) setjmp((ctx).buf)

#define save_context(tcb, ctx) (tcb).set_processor_state((ctx)), save_ctx(ctx)
