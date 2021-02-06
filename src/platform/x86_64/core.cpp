#include <tos/x86_64/assembly.hpp>

namespace tos::platform {
void force_reset() {
    x86_64::breakpoint();
    while (true);
}
}