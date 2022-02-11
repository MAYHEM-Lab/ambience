#include <tos/ae/kernel/group.hpp>
#include <tos/ae/kernel/platform_support.hpp>
#include <tos/ae/kernel/loaders/preemptive_elf_group.hpp>
#include <tos/ae/kernel/loading.hpp>
#include <registry.hpp>

// clang-format off
#include <{{group_name}}_elf.hpp>
// clang-format on

tos::Task<void> {{group_name}}_do_exports();

namespace {
struct descr {
    using loader = tos::ae::kernel::preemptive_elf_group;
    static constexpr std::string_view name = "{{group_name}}";
    static constexpr auto& elf_body = {{group_name}}_elf;
    static constexpr auto services = tos::meta::list<{{service_types | join("::service_type, ")}}::service_type > {};
};

struct meta : tos::ae::kernel::group_meta {
    std::string_view name() const override {
        return descr::name;
    }

    tos::Task<std::unique_ptr<tos::ae::kernel::group>> load(const platform_group_args& platform_args) override {
        auto group = tos::ae::kernel::load_it<descr>(platform_args);

        group->exposed_services.resize({{imported_services|length}});
        {% for service_name in imported_services %}
        group->exposed_services[{{imported_services[service_name]}}] =
            tos::ae::service_host(co_await registry.template wait<"{{service_name}}">());
        {% endfor %}

        {% for service_name in service_names %}
        registry.register_service("{{service_name}}", group->channels[{{loop.index}} - 1].get());
        {% endfor %}
        
        co_await {{group_name}}_do_exports();

        co_return group;
    }
};

meta group_meta;
} // namespace

using init_fn_t = void(*)();
[[gnu::section(".group_init.{{group_name}}"), gnu::used, gnu::retain]]
extern const init_fn_t {{group_name}}_init = +[]{
    LOG("Register {{group_name}}");
    tos::ae::kernel::known_groups.push_back(group_meta);
};