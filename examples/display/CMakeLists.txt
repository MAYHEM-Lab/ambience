add_executable(display_demo hosted.cpp)
target_link_libraries(display_demo PRIVATE tos_core arch_drivers tos_gfx tos_dynamic_log)