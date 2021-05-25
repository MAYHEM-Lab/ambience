#include <tos/processor_context.hpp>

namespace tos {
[[noreturn]] void switch_context(processor_context& j, context_codes rc) {
#if defined(TOS_ARCH_HAS_PROC_STATE)
    tos_longjmp(j.buf, static_cast<int>(rc));
#else
    longjmp(j.buf, static_cast<int>(rc));
#endif
    TOS_UNREACHABLE();
}

void swap_context(processor_context& save_to, processor_context& switch_to) {
    if (save_ctx(save_to) == context_codes::saved) {
        switch_context(switch_to, context_codes::scheduled);
    }
}
} // namespace tos