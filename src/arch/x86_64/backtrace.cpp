#include <tos/x86_64/assembly.hpp>
#include <tos/x86_64/backtrace.hpp>

namespace tos::x86_64 {
NO_INLINE
std::optional<trace_elem> backtrace_next(uint64_t rbp) {
    auto ptr = reinterpret_cast<const uint64_t*>(rbp);
    if (!ptr) {
        return std::nullopt;
    }

    // The ABI specifies RBP to be right after the return address.

    auto prev_rbp = *ptr;

    // Return address points to the next instruction.
    // It's not really possible to find the exact call instruction address so we just make
    // an "educated" guess about it. A call instruction is anywhere between 2 to 5 bytes.
    auto prev_rip = *(ptr + 1) - 4;

    return trace_elem{.rbp = prev_rbp, .rip = prev_rip};
}
} // namespace tos::x86_64