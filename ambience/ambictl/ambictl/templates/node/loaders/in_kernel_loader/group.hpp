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
    tos::Task<void> post_load();
};

auto init_{{group_name}}(const platform_group_args& platform_args) -> {{group_name}};
// clang-format on
