include(CMakeRC)

add_library(tos_mem include/tos/memory.hpp src/memory.cpp)
target_link_libraries(tos_mem PUBLIC tos_util_core)
target_include_directories(tos_mem PUBLIC include)

add_library(tos_memory
        include/tos/allocator/buddy.hpp
        src/allocator/buddy.cpp
        include/tos/allocator/bump.hpp
        src/allocator/free_list.cpp
        include/tos/allocator/polymorphic_allocator.hpp
        include/tos/allocator/malloc.hpp
        include/tos/allocator/null_allocator.hpp)
target_link_libraries(tos_memory PUBLIC tos_util_core tos_debug_log)
target_include_directories(tos_memory PUBLIC include)

if (BUILD_TESTS)
    enable_testing()
    add_executable(tos_memory_buddy_test src/allocator/buddy_test.cpp)
    target_link_libraries(tos_memory_buddy_test PUBLIC standalone_tests_main tos_memory)
    add_test(NAME tos_memory_buddy_test COMMAND tos_memory_buddy_test)

    add_executable(tos_memory_bump_test src/allocator/bump_test.cpp)
    target_link_libraries(tos_memory_bump_test PUBLIC standalone_tests_main tos_memory)
    add_test(NAME tos_memory_bump_test COMMAND tos_memory_bump_test)

    add_executable(tos_memory_free_list_test src/allocator/free_list_test.cpp)
    target_link_libraries(tos_memory_free_list_test PUBLIC standalone_tests_main tos_memory)
    add_test(NAME tos_memory_free_list_test COMMAND tos_memory_free_list_test)
endif()

add_library(tos_debug_log
        src/log.cpp
        include/tos/debug/log.hpp
        include/tos/debug/detail/logger_base.hpp
        include/tos/debug/detail/log_message.hpp
        include/tos/debug/sinks/serial_sink.hpp
        include/tos/debug/sinks/null_sink.hpp
        include/tos/debug/sinks/sink.hpp
        include/tos/debug/sinks/clock_adapter.hpp
        include/tos/debug/sinks/async_sink.hpp
)
target_include_directories(tos_debug_log PUBLIC include)
target_link_libraries(tos_debug_log PUBLIC tos_util_core tos_print fmt)

include(lidlc)

add_lidlc(log_schema log.yaml)
add_library(lidl_log_sink include/tos/debug/sinks/lidl_sink.hpp src/lidl_sink.cpp)
target_include_directories(tos_debug_log PUBLIC include)
target_link_libraries(lidl_log_sink PUBLIC tos_debug_log log_schema)

add_library(tos_dynamic_log src/dynamic_log.cpp include/tos/debug/dynamic_log.hpp)
target_link_libraries(tos_dynamic_log PUBLIC tos_debug_log)

add_library(tos_context
        src/context.cpp
        include/tos/context.hpp
        include/tos/components/name.hpp
        include/tos/components/component.hpp
        include/tos/components/allocator.hpp)
target_link_libraries(tos_context PUBLIC tos_util_core tos_arch_core tos_debug_log tos_platform)
target_include_directories(tos_context PUBLIC include)

if (BUILD_TESTS)
    add_executable(log_tests log_tests.cpp)
    target_link_libraries(log_tests PUBLIC tests_main tos_core tos_debug_log)
    add_test(NAME log_tests COMMAND log_tests)

    add_executable(context_tests src/context_test.cpp)
    target_link_libraries(context_tests PUBLIC tests_main tos_core)
    add_test(NAME context_tests COMMAND context_tests)
endif()

add_library(tos_debug_core
        include/tos/debug/assert.hpp
        src/debug/core_assert.cpp)
target_link_libraries(tos_debug_core PUBLIC tos_util_core tos_platform)
target_include_directories(tos_debug_core PUBLIC include)

add_library(tos_debug
        include/tos/debug/debug.hpp
        include/tos/debug/stack_dump.hpp
        include/tos/debug/panic.hpp
        src/debug.cpp
        src/panic.cpp
)
target_link_libraries(tos_debug PUBLIC tos_debug_core tos_util_core tos_debug_log tos_platform tos_ft)

add_library(tos_build_info
        include/tos/build.hpp
        src/build.cpp)
target_include_directories(tos_build_info PUBLIC include)
target_link_libraries(tos_build_info PUBLIC tos_util_core)
target_compile_definitions(tos_build_info PRIVATE TOS_CONFIG_PLATFORM=\"${TOS_PLATFORM_NAME}\")
target_compile_definitions(tos_build_info PRIVATE TOS_CONFIG_ARCH=\"${TOS_ARCH}\")
target_compile_definitions(tos_build_info PRIVATE TOS_CONFIG_DRIVERS=\"${TOS_DRIVERS}\")

include(GetGitRevisionDescription)
get_git_head_revision(GIT_REFSPEC GIT_SHA1)
target_compile_definitions(tos_build_info PRIVATE TOS_GIT_SHA1=\"${GIT_SHA1}\")

configure_file(arch.hpp.in include/tos/core/arch.hpp)
configure_file(arch_fwd.hpp.in include/tos/core/arch_fwd.hpp)
add_library(tos_arch_core src/stub.cpp)
target_link_libraries(tos_arch_core PUBLIC arch_${TOS_ARCH})
target_include_directories(tos_arch_core PUBLIC "${CMAKE_CURRENT_BINARY_DIR}/include")

add_library(tos_platform include/tos/arch.hpp src/stub.cpp)
target_link_libraries(tos_platform PUBLIC tos_arch_core ${TOS_PLATFORM_NAME}_core)
target_compile_definitions(tos_platform PUBLIC TOS_PLATFORM_${TOS_PLATFORM_NAME})
message(STATUS "Platform: ${TOS_PLATFORM_NAME}")

add_library(tos_interrupt include/tos/interrupt.hpp src/interrupt.cpp)
target_link_libraries(tos_interrupt PUBLIC tos_arch_core tos_util_core tos_platform tos_debug_core)

add_library(tos_jobs include/tos/scheduler.hpp include/tos/job.hpp src/job.cpp src/scheduler.cpp)
target_link_libraries(tos_jobs PUBLIC tos_util_core tos_context)
target_link_libraries(tos_jobs PRIVATE tos_interrupt)

add_library(tos_core
        include/tos/devices.hpp
        include/tos/char_stream.hpp
        include/tos/arch.hpp
        include/tos/device_id.hpp
        include/tos/driver_traits.hpp
        include/tos/delay.hpp
        include/tos/barrier.hpp
        src/tos_init.cpp
        src/device_id.cpp
        src/paging.cpp
        src/paging/physical_page_allocator.cpp
)
target_link_libraries(tos_core PUBLIC tos_mem tos_util tos_build_info ${TOS_PLATFORM_NAME}_impl)
target_link_libraries(tos_core PUBLIC tos::libcxx tos_debug tos_context)
target_link_libraries(tos_core PUBLIC cxxabi_static)
target_link_libraries(tos_core PUBLIC tos_ft tos_arch_core tos_interrupt tos_platform tos_jobs)
target_include_directories(tos_core PUBLIC include)
add_library(tos::core ALIAS tos_core)

add_library(tos_interrupt_trampoline include/tos/interrupt_trampoline.hpp src/interrupt_trampoline.cpp)
target_include_directories(tos_interrupt_trampoline PUBLIC include)
target_link_libraries(tos_interrupt_trampoline PUBLIC tos_ft tos_util_core)

add_library(tos_print include/tos/print.hpp src/print.cpp)
target_link_libraries(tos_print PUBLIC tos_util_core tos::libcxx)
add_library(tos::print ALIAS tos_print)

tos_install(tos_platform ${CMAKE_CURRENT_SOURCE_DIR}/include)
tos_install(tos_arch_core ${CMAKE_CURRENT_SOURCE_DIR}/include)
tos_install(tos_interrupt ${CMAKE_CURRENT_SOURCE_DIR}/include)
tos_install(tos_core ${CMAKE_CURRENT_SOURCE_DIR}/include)
tos_install(tos_print ${CMAKE_CURRENT_SOURCE_DIR}/include)
tos_install(tos_mem ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_subdirectory(benchmarks)
add_subdirectory(address_space)

if (TARGET tos_address_space)
    target_link_libraries(tos_core PUBLIC tos_address_space)
endif()