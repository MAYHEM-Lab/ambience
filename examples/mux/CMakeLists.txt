add_executable(mux mux.cpp)
target_link_libraries(mux PUBLIC tos_core arch_drivers tos_print tos_dynamic_log tos_shell)