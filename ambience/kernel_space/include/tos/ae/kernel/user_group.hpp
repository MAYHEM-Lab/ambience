#pragma once

#include <memory>
#include <nonstd/variant.hpp>
#include <tos/ae/kernel/group.hpp>
#include <tos/ae/kernel/rings.hpp>
#include <tos/ae/transport/downcall.hpp>
#include <tos/arch.hpp>
#include <tos/context.hpp>
#include <tos/self_pointing.hpp>
#include <tos/task.hpp>
#include <tos/tcb.hpp>

namespace tos::ae::kernel {
struct user_group : group {
    kernel_interface iface;
    kern::tcb* state;
};
} // namespace tos::ae::kernel