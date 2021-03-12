#pragma once

#include <tos/ae/user_space.hpp>

namespace tos::ae::detail {
void do_init_syscall(interface& iface);
void do_yield_syscall();
}