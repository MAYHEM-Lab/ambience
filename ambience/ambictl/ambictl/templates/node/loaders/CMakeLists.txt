{% for group in node_loaders %}
add_subdirectory({{group}})
{% endfor %}