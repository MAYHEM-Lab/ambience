add_library({{node_name}}_common INTERFACE)
target_include_directories({{node_name}}_common INTERFACE .)

{% for target in link_targets %}
target_link_libraries({{node_name}}_common INTERFACE {{target}})
{% endfor %}

{% for schema in schemas %}
target_link_libraries({{node_name}}_common INTERFACE {{schema}})
{% endfor %}

add_executable({{node_name}}_kernel groups.cpp)
target_link_libraries({{node_name}}_kernel PRIVATE tos_util_core tos_core tos_dynamic_log aekernel_node {{node_name}}_common)

add_subdirectory(loaders)

{% for group in node_groups %}
target_link_libraries({{node_name}}_kernel PRIVATE {{group}}_loader)
{% endfor %}

if (${TOS_ARCH} MATCHES "x86_64")
add_iso({{node_name}}-iso {{node_name}}_kernel)
endif()
