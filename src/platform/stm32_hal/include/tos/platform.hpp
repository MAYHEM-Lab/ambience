#pragma once

#include <tos/arm/interrupts.hpp>
#include <tos/arm/spmanip.hpp>

namespace tos::platform {
using arm::disable_interrupts;
using arm::enable_interrupts;

[[noreturn]]
void force_reset();
} // namespace tos::platform

#include <stm32_hal/hal.hpp>

namespace tos {
namespace stm32 {
extern uint32_t apb1_clock;
extern uint32_t ahb_clock;
} // namespace stm32
} // namespace tos