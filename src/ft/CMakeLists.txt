project(tos)

add_library(tos_ft
        include/tos/future.hpp
        include/tos/ft.hpp
        include/tos/tcb.hpp
        include/tos/mutex.hpp
        include/tos/semaphore.hpp
        include/tos/waitable.hpp
        include/tos/event.hpp
        src/ft.cpp
        src/semaphore.cpp
        include/tos/ft.inl
        include/tos/thread.hpp
        include/tos/types.hpp
        src/tcb.cpp
        include/tos/components/threads.hpp
        src/components/threads.cpp
        include/tos/suspended_launch.hpp
        src/waitable.cpp
        src/core_waitable.cpp
        src/condition_variable.cpp 
        src/shared_mutex.cpp
)

target_link_libraries(tos_ft PUBLIC tos_util_core tos_coro)
target_link_libraries(tos_ft PUBLIC tos_interrupt)
target_link_libraries(tos_ft PUBLIC tos::libcxx)
target_link_libraries(tos_ft PUBLIC tos_alarm tos_context tos_pcontext tos_fiber)

add_library(tos::ft ALIAS tos_ft)

tos_install(tos_ft ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_subdirectory(benchmarks)