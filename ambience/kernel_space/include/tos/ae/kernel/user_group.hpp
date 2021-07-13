#pragma once

#include <tos/ae/kernel/group.hpp>
#include <tos/ae/kernel/rings.hpp>
#include <tos/tcb.hpp>

namespace tos::ae::kernel {
struct user_group : group {
    kernel_interface iface;
    kern::tcb* state;
    bool m_runnable = false;

    void notify_downcall();
};
} // namespace tos::ae::kernel