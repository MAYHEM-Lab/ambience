#include <asm.hpp>

extern "C" {
void tos_force_reset() {
    tos::aarch64::intrin::bkpt();
    while (true)
        ;
}
}