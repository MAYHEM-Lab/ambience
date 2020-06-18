#include <tos/aarch64/assembly.hpp>

namespace tos::platform {
[[noreturn]] void force_reset() {
    tos::aarch64::breakpoint();
    while (true)
        ;
}
} // namespace tos::platform