#pragma once

#include <cstdint>
#include <optional>
#include <tos/compiler.hpp>

namespace tos::x86_64 {
struct trace_elem {
    uint64_t rbp;
    uint64_t rip;
};

NO_INLINE
std::optional<trace_elem> backtrace_next(uint64_t rbp);

trace_elem backtrace_current();

inline std::optional<trace_elem> backtrace_next(const trace_elem& cur) {
    return backtrace_next(cur.rbp);
}
} // namespace tos::x86_64