add_library(tos_coro src/coro.cpp include/tos/task.hpp include/tos/detail/coro.hpp include/tos/detail/poll.hpp)
target_link_libraries(tos_coro PUBLIC tos_util_core)
target_include_directories(tos_coro PUBLIC include)

add_library(tos_coro_bind src/coro_bind.cpp include/tos/detail/tos_bind.hpp)
target_link_libraries(tos_coro_bind PUBLIC tos_coro tos_jobs)

if (${CMAKE_CXX_COMPILER_ID} MATCHES "GNU")
    target_compile_options(tos_coro PUBLIC $<$<COMPILE_LANGUAGE:CXX>:-fcoroutines>)
endif()

if(BUILD_TESTS)
    add_executable(coro_test src/coro_test.cpp)
    target_link_libraries(coro_test PUBLIC standalone_tests_main tos_coro)
    add_test(NAME coro_test COMMAND coro_test)
endif()