#pragma once

#include <tos/x86_64/interrupts.hpp>
#include <tos/x86_64/spmanip.hpp>

namespace tos::platform {
using x86_64::interrupts_disabled;
using x86_64::disable_interrupts;
using x86_64::enable_interrupts;

[[noreturn]]
void force_reset();
} // namespace tos::platform
