include(lidlc)

add_lidlc(storage_schema storage.yaml)

add_executable(storage_demo main.cpp)
target_link_libraries(storage_demo PRIVATE tos_core tos_board tos_dynamic_log storage_schema)