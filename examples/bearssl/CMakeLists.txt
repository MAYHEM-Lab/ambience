
add_executable(bear main.cpp keys.hpp)
target_link_libraries(bear PUBLIC tos_core arch_drivers tos::core tos::print caps BearSSL::BearSSL)
