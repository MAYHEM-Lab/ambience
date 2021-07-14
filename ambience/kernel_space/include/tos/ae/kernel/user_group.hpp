#pragma once

#include <tos/ae/kernel/group.hpp>
#include <tos/ae/kernel/rings.hpp>
#include <tos/tcb.hpp>

namespace tos::ae::kernel {
struct user_group : group {
    kernel_interface iface;
    kern::tcb* state;

    void notify_downcall();
    void clear_runnable();
private:
    bool m_runnable = false;
};
} // namespace tos::ae::kernel