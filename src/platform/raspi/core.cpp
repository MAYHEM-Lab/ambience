#include <tos/aarch64/assembly.hpp>

extern "C" {
void tos_force_reset() {
    tos::aarch64::breakpoint();
    while (true)
        ;
}
}