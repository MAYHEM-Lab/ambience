add_executable(x86_timer_demo main.cpp)
target_link_libraries(x86_timer_demo PUBLIC tos::core x86_drivers)