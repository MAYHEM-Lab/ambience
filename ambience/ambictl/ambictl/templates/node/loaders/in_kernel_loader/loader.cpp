#include <registry.hpp>
#include <tos/ae/kernel/group.hpp>

{% for include in init_includes %}
#include <{{include}}>
{% endfor %}

{{service_init_sigs}}

tos::Task<void> {{group_name}}_do_exports();

namespace {
struct meta : tos::ae::kernel::group_meta {
    std::string_view name() const override {
        return "{{group_name}}";
    }

    tos::Task<std::unique_ptr<tos::ae::kernel::group>> load(const platform_group_args& platform_args) override {
        {% for init in service_inits %}
        {{init}}
        {% endfor %}
        
        co_await {{group_name}}_do_exports();

        co_return nullptr;
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