if (${TOS_ARCH} MATCHES "i386")
    include(AddIso)

    add_i386_loader({{node_name}}-loader ${CMAKE_BINARY_DIR}/../cmake-build-barex64 {{node_name}}_kernel)
    add_iso({{node_name}}-iso {{node_name}}-loader)
    return()
endif()

add_library({{node_name}}_common INTERFACE)
target_include_directories({{node_name}}_common INTERFACE .)

{% for schema in schemas %}
target_link_libraries({{node_name}}_common INTERFACE {{schema}})
{% endfor %}

add_executable({{node_name}}_kernel groups.cpp)
target_link_libraries({{node_name}}_kernel PRIVATE tos_util_core tos_core tos_dynamic_log aekernel_node {{node_name}}_common)

add_subdirectory(groups)

{% for group in node_groups %}
target_link_libraries({{node_name}}_kernel PRIVATE {{group}}_loader)
{% endfor %}