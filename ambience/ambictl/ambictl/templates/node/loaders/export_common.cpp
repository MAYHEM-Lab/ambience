#include <registry.hpp>

tos::Task<void> {{group_name}}_do_exports() {
    {% for export_string in export_strings %}
    {{export_string}}
    {% endfor %}
    co_return;
}