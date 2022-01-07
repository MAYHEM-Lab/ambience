#include "tos/compiler.hpp"
#include <tos/arch.hpp>
#include <tos/debug/panic.hpp>

namespace tos::debug {
TOS_NO_OPTIMIZE
[[noreturn]] void panic(const char* err) {
    tos::cur_arch::nop();
    tos::debug::fatal(err);
    tos::platform::force_reset();
}
} // namespace tos::debug