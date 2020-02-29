#include <arch/interrupts.hpp>
#include <asm.hpp>

namespace tos::raspi3 {
namespace {
void reset_handler() {
    aarch64::intrin::bkpt();
}

void undefined_instruction_handler() {
}

void swi_handler() {
}

void prefetch_abort_handler() {
}

void data_abort_handler() {
}

void irq_handler() {
}

void fiq_handler() {
}
} // namespace
} // namespace tos::raspi3