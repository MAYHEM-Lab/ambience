add_library({{group_name}}_lib group.cpp interface.cpp)
target_link_libraries({{group_name}}_lib PUBLIC ae_user_space)

{% for target in service_targets %}
target_link_libraries({{group_name}}_lib PUBLIC {{target}})
{% endfor %}
{% for schema in schemas %}
target_link_libraries({{group_name}}_lib PUBLIC {{schema}})
{% endfor %}

add_executable({{group_name}}_size null.cpp)
target_link_libraries({{group_name}}_size PUBLIC {{group_name}}_lib)
add_executable({{group_name}} null.cpp)
target_link_libraries({{group_name}} PUBLIC {{group_name}}_lib)

if (NOT ${TOS_ARCH} MATCHES "wasm")
target_link_libraries({{group_name}}_size PUBLIC "-T size_compute.ld")
target_link_libraries({{group_name}} PUBLIC "-T ${CMAKE_CURRENT_SOURCE_DIR}/linker.ld")
endif()
