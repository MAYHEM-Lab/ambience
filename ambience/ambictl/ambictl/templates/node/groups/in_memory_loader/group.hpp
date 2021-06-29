#pragma once

#include <tos/ae/kernel/user_group.hpp>
#include <tos/ae/service_host.hpp>
#include <tos/arch.hpp>
#include <tos/interrupt_trampoline.hpp>
#include <tos/paging/physical_page_allocator.hpp>
#include <tos/ae/kernel/platform_support.hpp>
#include <tos/debug/assert.hpp>

// clang-format off
{% for include in service_includes %}
#include <{{include}}>
{% endfor %}

struct {{group_name}} {
std::unique_ptr<tos::ae::kernel::user_group> group;

{% for service_name in services %}
auto {{service_name}}() -> auto& {
return static_cast<{{services[service_name]}}&>(
    *group->channels[{{loop.index}} - 1]);
}
{% endfor %}

template<class Registry>
tos::Task<void> init_dependencies(Registry& registry) {
    group->exposed_services.resize({{imported_services|length}});
    {% for service_name in imported_services %}
    group->exposed_services[{{imported_services[service_name]}}] =
        tos::ae::service_host(co_await registry.template wait<"{{service_name}}">());
    {% endfor %}

    // Wait for all dependencies to come online, then register our services
    {% for service_name in services %}
    registry.template register_service<"{{service_name}}">(&{{service_name}}());
    {% endfor %}
}
};

auto init_{{group_name}}(const platform_group_args& platform_args) -> {{group_name}};
// clang-format on
