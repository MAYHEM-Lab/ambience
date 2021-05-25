#pragma once

#include <memory>
#include <nonstd/variant.hpp>
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
    std::vector<mpark::variant<mpark::monostate,
                               tos::ae::sync_service_host,
                               tos::ae::async_service_host>>
        exposed_services;
};
} // namespace tos::ae::kernel