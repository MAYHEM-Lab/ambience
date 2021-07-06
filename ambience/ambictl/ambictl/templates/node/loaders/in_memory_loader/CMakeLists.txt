add_library({{group_name}}_loader loader.cpp exports.cpp {{group_name}}.hpp)
target_link_libraries({{group_name}}_loader PUBLIC tos_ft aekernel {{node_name}}_common)
target_include_directories({{group_name}}_loader PUBLIC .)

{% for schema in schemas %}
target_link_libraries({{group_name}}_loader PUBLIC {{schema}})
{% endfor %}
