#pragma once

#include <tos/ae/registry.hpp>

// clang-format off
{% for include in service_includes %}
#include <{{include}}>
{% endfor %}
// clang-format on

using registry_t = tos::ae::service_registry<
//clang-format off
{% for service_name in services %}
    tos::ae::service_mapping<"{{service_name}}", {{services[service_name]}}*>
    {% if not loop.last %}
    ,
    {% endif %}
{% endfor %}
//clang-format on
>;

extern registry_t registry;
tos::ae::registry_base& get_registry();
