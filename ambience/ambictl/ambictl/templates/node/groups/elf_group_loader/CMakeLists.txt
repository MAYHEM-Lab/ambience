add_library({{group_name}}_loader loader.cpp {{group_name}}.hpp)
target_link_libraries({{group_name}}_loader PUBLIC tos_elf aekernellib {{node_name}}_common)
target_include_directories({{group_name}}_loader PUBLIC .)

{% for schema in schemas %}
target_link_libraries({{group_name}}_loader PUBLIC {{schema}})
{% endfor %}

include(FileEmbed)

add_dependencies({{group_name}}_loader {{group_name}})
embed_file({{group_name}}_loader {{group_name}}_elf $<TARGET_FILE:{{group_name}}>.stripped)