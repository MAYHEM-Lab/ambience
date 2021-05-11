#pragma once

#include <tos/ae/kernel/rings.hpp>
#include <tos/ae/service_host.hpp>
#include <tos/context.hpp>
#include <tos/task.hpp>
#include <tos/tcb.hpp>

namespace tos::ae::kernel {
struct user_group {
    kernel_interface iface;
    kern::tcb* state;
    std::vector<std::unique_ptr<lidl::service_base>> channels;
    std::vector<tos::ae::service_host> exposed_services;
};
} // namespace tos::ae::kernel