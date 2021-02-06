#pragma once

#include <tos/avr/spmanip.hpp>
#include <tos/avr/interrupts.hpp>

namespace tos::platform {
using avr::interrupts_disabled;
using avr::enable_interrupts;
using avr::disable_interrupts;

[[noreturn]]
void force_reset();
}