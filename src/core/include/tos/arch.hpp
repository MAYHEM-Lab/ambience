//
// Created by fatih on 4/16/18.
//

#pragma once

#include <tos/compiler.hpp>
#include <tos/platform.hpp>

extern "C" {
[[noreturn]] void tos_force_reset();
}

namespace tos::arch {
using platform::arch::set_stack_ptr;
using platform::arch::get_stack_ptr;
}