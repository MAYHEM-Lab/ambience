#include <tos/ae/kernel/loaders/in_memory_user.hpp>
#include <tos/ae/kernel/loading.hpp>
#include <{{group_name}}.hpp>
#include <registry.hpp>

// clang-format off
{% for include in service_includes %}
#include <{{include}}>
{% endfor %}
// clang-format on

tos::Task<void> {{group_name}}::post_load() {
    co_return;
}

auto init_{{group_name}}(const platform_group_args& platform_args) -> {{group_name}} {
    return {};
}
