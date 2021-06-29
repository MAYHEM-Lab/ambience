#include <tos/ae/kernel/loaders/preemptive_elf_group.hpp>
#include <tos/ae/kernel/loading.hpp>
#include <{{group_name}}.hpp>

// clang-format off
#include <{{group_name}}_elf.hpp>
{% for include in service_includes %}
#include <{{include}}>
{% endfor %}
// clang-format on

namespace {
struct descr {
    using loader = tos::ae::kernel::preemptive_elf_group;
    static constexpr auto& elf_body = {{group_name}}_elf;
    static constexpr auto services = tos::meta::list<{{services | join("::service_type, ")}}::service_type > {};
};
} // namespace

auto init_{{group_name}}(const platform_group_args& platform_args) -> {{group_name}} {
    return {{group_name}} {tos::ae::kernel::load_it<descr>(platform_args)};
}