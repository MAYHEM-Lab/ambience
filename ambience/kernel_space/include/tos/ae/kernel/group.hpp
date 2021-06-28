#pragma once

#include <memory>
#include <tos/ae/kernel/runners/group_runner.hpp>
#include <tos/ae/service_host.hpp>
#include <tos/intrusive_list.hpp>
#include <vector>

namespace tos::ae::kernel {
struct group : list_node<group> {
    group_runner* runner;
    // Services exported by this group
    std::vector<std::unique_ptr<lidl::service_base>> channels;

    // Services imported by this group
    std::vector<mpark::variant<mpark::monostate,
                               tos::ae::sync_service_host,
                               tos::ae::async_service_host>>
        exposed_services;

    void run() {
        runner->run(*this);
    }
};
} // namespace tos::ae::kernel