{% for group in node_groups %}
add_subdirectory({{group}})
{% endfor %}