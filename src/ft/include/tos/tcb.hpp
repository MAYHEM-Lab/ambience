#pragma once

#include <algorithm>
#include <cstddef>
#include <setjmp.h>
#include <tos/arch.hpp>
#include <tos/intrusive_list.hpp>
#include <tos/utility.hpp>
#include <utility>

namespace tos {
namespace kern {
struct ctx;

/**
 * This type represents an execution context in the system.
 *
 * It's expected that a concrete task descriptor type will
 * extend this class to implement required functionality such
 * as starting threads or passing arguments.
 */
struct alignas(alignof(std::max_align_t)) tcb : public list_node<tcb> {
    /**
     * Returns a reference to the context of the task.
     *
     * The function can either be called to store the current
     * context, or to resume execution from the context.
     *
     * @return execution context of the task
     */
    ctx& get_ctx() {
        return *m_ctx;
    }

    void set_ctx(ctx& buf) {
        m_ctx = &buf;
    }

    /**
     * The threading subsystem does not know about the concrete time
     * at the destruction of a task, so the destructor must be virtual
     * in order to properly destroy the concrete task descriptor.
     */
    virtual ~tcb() = 0;

private:
    ctx* m_ctx;
};

inline tcb::~tcb() = default;
} // namespace kern
} // namespace tos

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
struct ctx {
    jmp_buf buf;
};

[[noreturn]] inline void switch_context(kern::ctx& j, return_codes rc) {
    longjmp(j.buf, static_cast<int>(rc));

    __builtin_unreachable();
}
} // namespace kern
} // namespace tos

#define save_ctx(ctx) (::tos::return_codes) setjmp((ctx).buf)

#define save_context(tcb, ctx) (tcb).set_ctx((ctx)), save_ctx(ctx)
