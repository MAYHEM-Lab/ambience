#include <{{group_name}}.hpp>
#include <registry.hpp>

tos::Task<void> {{group_name}}::do_exports() {
    {% for export_string in export_strings %}
    {{export_string}}
    {% endfor %}
    co_return;
}