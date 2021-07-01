#include <tos/ae/kernel/loaders/in_memory_user.hpp>
#include <tos/ae/kernel/loading.hpp>
#include <{{group_name}}.hpp>
#include <registry.hpp>

// clang-format off
{% for include in service_includes %}
#include <{{include}}>
{% endfor %}
// clang-format on

namespace {
struct descr {
    using loader = tos::ae::kernel::in_memory_group;
    static constexpr auto start_address = {{start_addr}};
    static constexpr auto services = tos::meta::list<{{services | join("::service_type, ")}}::service_type > {};
};
} // namespace


tos::Task<void> {{group_name}}::post_load() {
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

auto init_{{group_name}}(const platform_group_args& platform_args) -> {{group_name}} {
    return {{group_name}} {tos::ae::kernel::load_it<descr>(platform_args)};
}