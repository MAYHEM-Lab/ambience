#pragma once

#include <tos/aarch64/interrupts.hpp>
#include <tos/aarch64/spmanip.hpp>

namespace tos::platform {
using aarch64::disable_interrupts;
using aarch64::enable_interrupts;
namespace arch = aarch64;
} // namespace tos::platform
