#include <{{group_name}}.hpp>
#include <registry.hpp>


{% for include in init_includes %}
#include <{{include}}>
{% endfor %}

{{service_init_sigs}}
tos::Task<void> {{group_name}}::post_load() {
    {% for init in service_inits %}
    {{init}}
    {% endfor %}

    co_await do_exports();
}

auto init_{{group_name}}(const platform_group_args& platform_args) -> {{group_name}} {
    return {};
}
