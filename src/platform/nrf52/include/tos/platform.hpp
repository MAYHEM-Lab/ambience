#pragma once

#include <tos/arm/interrupts.hpp>
#include <tos/arm/spmanip.hpp>
#include <tos/debug/trace/metrics/counter.hpp>

namespace tos::platform {
using arm::disable_interrupts;
using arm::enable_interrupts;
using arm::interrupts_disabled;

[[noreturn]] void force_reset();
} // namespace tos::platform

extern tos::trace::basic_counter systicks;