#pragma once

#include <memory>
#include <tos/ae/kernel/runners/group_runner.hpp>
#include <tos/ae/service_host.hpp>
#include <tos/intrusive_list.hpp>
#include <vector>
#include <tos/debug/log.hpp>

struct platform_group_args;

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

struct group_meta : list_node<group_meta> {
    virtual std::string_view name() const = 0;

    virtual Task<std::unique_ptr<group>> load(const platform_group_args&) = 0;

    virtual ~group_meta() = default;
};

inline intrusive_list<group_meta> known_groups{};
} // namespace tos::ae::kernel