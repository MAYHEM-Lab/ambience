#pragma once

#include <tos/arch.hpp>

namespace tos {
enum class context_codes : uint8_t
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

struct processor_context {
#if defined(TOS_ARCH_HAS_PROC_STATE)
    tos::cur_arch::proc_state_t buf;
#else
    jmp_buf buf;
#endif
};

[[noreturn]] void switch_context(processor_context& j, context_codes rc);
void swap_context(processor_context& save_to, processor_context& switch_to);
} // namespace tos

#if defined(TOS_ARCH_HAS_PROC_STATE)
#define save_ctx(ctx) (::tos::context_codes) tos_setjmp((ctx).buf)
#else
#define save_ctx(ctx) (::tos::context_codes) setjmp((ctx).buf)
#endif
