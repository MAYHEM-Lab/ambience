project(tos)

add_library(tos_ft
        include/tos/ft.hpp
        include/tos/thread_info.hpp
        include/tos/mutex.hpp
        include/tos/semaphore.hpp
        include/tos/scheduler.hpp
        include/tos/waitable.hpp
        src/ft.cpp
        src/semaphore.cpp)

target_include_directories(tos_ft PUBLIC "include")
target_compile_options(tos_ft PUBLIC -std=gnu++14)

target_link_libraries(tos_ft PUBLIC tos_arch)
target_link_libraries(tos_ft PUBLIC tos_util)
target_link_libraries(tos_ft PUBLIC tos_interrupt)
target_compile_definitions(tos_ft PUBLIC TOS=ON)
