#include <{{group_name}}.hpp>
#include <registry.hpp>

// clang-format off
{% for include in export_includes %}
#include <{{include}}>
{% endfor %}
// clang-format on

tos::Task<void> {{group_name}}::do_exports() {
    {% for export_string in export_strings %}
    {{export_string}}
    {% endfor %}
}