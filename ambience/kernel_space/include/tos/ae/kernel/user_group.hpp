#pragma once

#include <tos/ae/kernel/rings.hpp>
#include <tos/context.hpp>
#include <tos/task.hpp>
#include <tos/tcb.hpp>

namespace tos::ae::kernel {
struct user_group {
    kernel_interface iface;
    kern::tcb* state;
};
} // namespace tos::ae::kernel