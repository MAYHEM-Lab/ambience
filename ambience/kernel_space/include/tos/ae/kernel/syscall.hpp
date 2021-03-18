#pragma once

#include <tos/ae/kernel/user_group.hpp>
#include <tos/ae/rings.hpp>
#include <tos/function_ref.hpp>

namespace tos::ae::kernel {
using init_syscall_handler = function_ref<void(user_group& group, interface& iface)>;
using yield_handler = function_ref<void(user_group& group)>;
}