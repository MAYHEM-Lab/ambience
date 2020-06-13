#pragma once

#include <tos/arm/spmanip.hpp>

namespace tos::platform {
void enable_interrupts();
void disable_interrupts();
namespace arch = arm;
} // namespace tos::platform