add_executable(portable_gpio main.cpp)
target_link_libraries(portable_gpio PUBLIC tos_core)
target_compile_options(portable_gpio PRIVATE "-fwhole-program")