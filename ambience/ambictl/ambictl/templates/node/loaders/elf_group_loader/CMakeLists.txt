add_library({{group_name}}_loader loader.cpp exports.cpp)
target_link_libraries({{group_name}}_loader PUBLIC tos_elf tos_ft aekernel {{node_name}}_common ae_usergroup)
target_include_directories({{group_name}}_loader PUBLIC .)

{% for schema in schemas %}
target_link_libraries({{group_name}}_loader PUBLIC {{schema}})
{% endfor %}

include(CrossCmake)
build_other_target({{group_name}}_loader "{{group_build_dir}}" {{group_name}})

include(FileEmbed)
embed_file({{group_name}}_loader {{group_name}}_elf "{{group_build_dir}}/bin/{{group_name}}.stripped")
