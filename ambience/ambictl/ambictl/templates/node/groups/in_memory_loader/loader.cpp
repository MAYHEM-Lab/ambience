#include <tos/ae/kernel/loaders/in_memory_user.hpp>
#include <tos/ae/kernel/loading.hpp>
#include <{{group_name}}.hpp>

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

auto init_{{group_name}}(const platform_group_args& platform_args) -> {{group_name}} {
return {{group_name}} {tos::ae::kernel::load_it<descr>(platform_args)};
}