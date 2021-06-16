add_executable(wasm3_demo main.cpp)
target_link_libraries(wasm3_demo PRIVATE tos_core m3 tos_board policy_schema)