#pragma once

#include <tos/arm/spmanip.hpp>

namespace tos::platform {
void enable_interrupts();
void disable_interrupts();

[[noreturn]]
void force_reset();
} // namespace tos::platform