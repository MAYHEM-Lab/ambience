project(tos)

add_library(tos_ft
        include/tos/ft.hpp
        include/tos/tcb.hpp
        include/tos/mutex.hpp
        include/tos/semaphore.hpp
        include/tos/scheduler.hpp
        include/tos/waitable.hpp
        include/tos/event.hpp
        src/ft.cpp
        src/semaphore.cpp
        include/tos/unwind.hpp
        include/tos/ft.inl
        include/tos/thread.hpp
        include/tos/types.hpp)

target_include_directories(tos_ft PUBLIC "include")

target_link_libraries(tos_ft PUBLIC tos_arch)
target_link_libraries(tos_ft PUBLIC tos_util)
target_link_libraries(tos_ft PUBLIC tos_interrupt)
target_link_libraries(tos_ft PUBLIC tos_core tos::libcxx)
target_compile_definitions(tos_ft PUBLIC TOS)

add_library(tos::ft ALIAS tos_ft)