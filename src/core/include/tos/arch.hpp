//
// Created by fatih on 4/16/18.
//

#pragma once

#include <tos/compiler.hpp>

extern "C" {
[[noreturn]] void tos_force_reset();
}
#include <tos/core/arch.hpp>

namespace tos::arch {
using cur_arch::set_stack_ptr;
using cur_arch::get_stack_ptr;
}