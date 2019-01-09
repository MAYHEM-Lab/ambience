
add_executable(capabilities main.cpp)
target_link_libraries(capabilities tos::core tos::print caps nlohmann::json)