#pragma once

#include <algorithm>
#include <cstddef>
#include <tos/arch.hpp>
#include <tos/context.hpp>
#include <tos/interrupt.hpp>
#include <tos/intrusive_list.hpp>
#include <tos/job.hpp>
#include <tos/utility.hpp>
#include <utility>

namespace tos::kern {
struct processor_state;

/**
 * This type represents an execution context in the system.
 *
 * It's expected that a concrete task descriptor type will
 * extend this class to implement required functionality such
 * as starting threads or passing arguments.
 */
struct alignas(alignof(std::max_align_t)) tcb : public job {
    explicit tcb(context& ctx);
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

#define TOS_FEATURE_TCB_HAVE_LOG_BLOCK_POINT
#ifdef TOS_FEATURE_TCB_HAVE_LOG_BLOCK_POINT
    bool log_block_point = false;
#endif

#define TOS_FEATURE_TCB_HAVE_NAME
#ifdef TOS_FEATURE_TCB_HAVE_NAME
    std::string_view name = "<no-name>";
#endif

    tos::list_node<tcb> m_siblings;

protected:
    void on_set_context(context& new_ctx) override;

public:
    void operator()() override;

private:
    processor_state* m_ctx;
};

inline void set_name(tcb& t, std::string_view name) {
#ifdef TOS_FEATURE_TCB_HAVE_NAME
    t.name = name;
#endif
}

inline std::string_view get_name(const tcb& t) {
#ifdef TOS_FEATURE_TCB_HAVE_NAME
    return t.name;
#else
    return "<names-not-enabled>";
#endif
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
#if defined(TOS_ARCH_HAS_PROC_STATE)
    tos::cur_arch::proc_state_t buf;
#else
    jmp_buf buf;
#endif
};

[[noreturn]] inline void switch_context(kern::processor_state& j, return_codes rc) {
#if defined(TOS_ARCH_HAS_PROC_STATE)
    tos_longjmp(j.buf, static_cast<int>(rc));
#else
    longjmp(j.buf, static_cast<int>(rc));
#endif
    TOS_UNREACHABLE();
}
} // namespace kern
void swap_context(kern::tcb& current, kern::tcb& to, const no_interrupts&);
} // namespace tos

#if defined(TOS_ARCH_HAS_PROC_STATE)
#define save_ctx(ctx) (::tos::return_codes) tos_setjmp((ctx).buf)
#else
#define save_ctx(ctx) (::tos::return_codes) setjmp((ctx).buf)
#endif

#define save_context(tcb, ctx) (tcb).set_processor_state((ctx)), save_ctx(ctx)
