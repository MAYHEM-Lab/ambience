#pragma once

#include <tos/arm/spmanip.hpp>
#include <tos/arm/interrupts.hpp>

namespace tos::platform {
using arm::interrupts_disabled;
using arm::enable_interrupts;
using arm::disable_interrupts;

[[noreturn]]
void force_reset();
}