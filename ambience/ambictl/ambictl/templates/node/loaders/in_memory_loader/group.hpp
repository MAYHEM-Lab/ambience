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

tos::Task<void> post_load();
};

auto init_{{group_name}}(const platform_group_args& platform_args) -> {{group_name}};
// clang-format on
