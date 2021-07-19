#include <tos/ae/kernel/loaders/preemptive_elf_group.hpp>
#include <tos/ae/kernel/loading.hpp>
#include <{{group_name}}.hpp>
#include <registry.hpp>

// clang-format off
#include <{{group_name}}_elf.hpp>
// clang-format on

namespace {
struct descr {
    using loader = tos::ae::kernel::preemptive_elf_group;
    static constexpr std::string_view name = "{{group_name}}";
    static constexpr auto& elf_body = {{group_name}}_elf;
    static constexpr auto services = tos::meta::list<{{service_types | join("::service_type, ")}}::service_type > {};
};
} // namespace

tos::Task<void> {{group_name}}::post_load() {
    group->exposed_services.resize({{imported_services|length}});
    {% for service_name in imported_services %}
    group->exposed_services[{{imported_services[service_name]}}] =
    tos::ae::service_host(co_await registry.template wait<"{{service_name}}">());
    {% endfor %}

    // Wait for all dependencies to come online, then register our services
    {% for service_name in service_names %}
    registry.template register_service<"{{service_name}}">(&{{service_name}}());
    {% endfor %}

    co_await do_exports();
}

auto init_{{group_name}}(const platform_group_args& platform_args) -> {{group_name}} {
    return {{group_name}} {tos::ae::kernel::load_it<descr>(platform_args)};
}