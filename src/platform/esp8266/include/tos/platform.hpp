#pragma once

#include <tos/xtensa/spmanip.hpp>
#include <tos/xtensa/interrupts.hpp>

namespace tos::platform {
using xtensa::enable_interrupts;
using xtensa::disable_interrupts;

[[noreturn]]
void force_reset();
}

namespace tos::esp82 {
static constexpr auto main_task_prio = 0;
}