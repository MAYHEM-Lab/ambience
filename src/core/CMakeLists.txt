include(CMakeRC)

include(GetGitRevisionDescription)
get_git_head_revision(GIT_REFSPEC GIT_SHA1)

add_library(tos_mem include/tos/memory.hpp src/memory.cpp)

add_library(tos_memory include/tos/memory/buddy.hpp src/memory/buddy.cpp include/tos/memory/bump.hpp src/memory/free_list.cpp)
target_link_libraries(tos_memory PUBLIC tos_util_core)
target_include_directories(tos_memory PUBLIC include)

if (BUILD_TESTS)
    enable_testing()
    add_executable(tos_memory_buddy_test src/memory/buddy_test.cpp)
    target_link_libraries(tos_memory_buddy_test PUBLIC tests_main tos_memory)
    add_test(tos_memory_buddy_test tos_memory_buddy_test)

    add_executable(tos_memory_bump_test src/memory/bump_test.cpp)
    target_link_libraries(tos_memory_bump_test PUBLIC tests_main tos_memory)
    add_test(tos_memory_bump_test tos_memory_bump_test)

    add_executable(tos_memory_free_list_test src/memory/free_list_test.cpp)
    target_link_libraries(tos_memory_free_list_test PUBLIC tests_main tos_memory)
    add_test(tos_memory_free_list_test tos_memory_free_list_test)
endif()

add_library(tos_debug_log
        src/log.cpp
        include/tos/debug/log.hpp
        include/tos/debug/detail/logger_base.hpp
        include/tos/debug/detail/log_message.hpp
        include/tos/debug/sinks/serial_sink.hpp
        include/tos/debug/sinks/null_sink.hpp
        include/tos/debug/sinks/sink.hpp include/tos/debug/sinks/clock_adapter.hpp)
target_include_directories(tos_debug_log PUBLIC include)
target_link_libraries(tos_debug_log PUBLIC tos_util_core tos_print)

add_library(tos_dynamic_log src/dynamic_log.cpp include/tos/debug/dynamic_log.hpp)
target_link_libraries(tos_dynamic_log PUBLIC tos_debug_log)

if (BUILD_TESTS)
    add_executable(log_tests log_tests.cpp)
    target_link_libraries(log_tests PUBLIC tests_main tos_core tos_debug_log)
    add_test(NAME log_tests COMMAND log_tests)
endif()

set(TOS_CORE_HEADERS
        include/tos/devices.hpp
        include/tos/char_stream.hpp
        include/tos/arch.hpp
        include/tos/version.hpp
        include/tos/debug/debug.hpp
        include/tos/device_id.hpp
        include/tos/driver_traits.hpp
        include/tos/delay.hpp
        include/tos/barrier.hpp
        include/tos/debug/stack_dump.hpp
        include/tos/debug/panic.hpp)

add_library(tos_print include/tos/print.hpp src/print.cpp)
target_link_libraries(tos_print PUBLIC tos_util tos::libcxx)
add_library(tos::print ALIAS tos_print)

add_library(tos_core src/stub.cpp ${TOS_CORE_HEADERS} src/tos_init.cpp src/device_id.cpp src/debug.cpp)
target_compile_definitions(tos_core PUBLIC TOS_GIT_SHA1=\"${GIT_SHA1}\")
target_link_libraries(tos_core PUBLIC tos_mem tos_util)
target_link_libraries(tos_core PUBLIC tos::libcxx tos_debug_log)
add_library(tos::core ALIAS tos_core)

add_library(tos_arch_core src/stub.cpp)
target_link_libraries(tos_arch_core PUBLIC ${TOS_ARCH_NAME}_core)

add_library(tos_arch include/tos/arch.hpp src/stub.cpp)
target_link_libraries(tos_arch PUBLIC ${TOS_ARCH_NAME}_impl)
message(STATUS ${TOS_ARCH_NAME}_impl)

add_library(tos_interrupt include/tos/interrupt.hpp src/interrupt.cpp)
target_link_libraries(tos_interrupt PUBLIC tos_arch_core tos_util_core)

target_link_libraries(tos_core PUBLIC tos_ft tos_arch_core tos_interrupt tos_arch)
target_compile_definitions(tos_core PUBLIC TOS_ARCH_${TOS_ARCH_NAME})
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    target_compile_options(tos_core PUBLIC -fdiagnostics-color=always)
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    target_compile_options(tos_core PUBLIC -fdiagnostics-color)
endif ()

tos_install(tos_arch ${CMAKE_CURRENT_SOURCE_DIR}/include)
tos_install(tos_arch_core ${CMAKE_CURRENT_SOURCE_DIR}/include)
tos_install(tos_interrupt ${CMAKE_CURRENT_SOURCE_DIR}/include)
tos_install(tos_core ${CMAKE_CURRENT_SOURCE_DIR}/include)
tos_install(tos_print ${CMAKE_CURRENT_SOURCE_DIR}/include)
tos_install(tos_mem ${CMAKE_CURRENT_SOURCE_DIR}/include)
