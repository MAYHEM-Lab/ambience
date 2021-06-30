set(TOS_PROJECT_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/tos)
{{tos_cmake_contents}}

{% for subdir in group_subdirs %}
add_subdirectory({{subdir}})
{% endfor %}

{% for subdir in node_subdirs %}
add_subdirectory({{subdir}})
{% endfor %}
