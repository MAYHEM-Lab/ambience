if (NOT TARGET bench_main)
    add_executable(bench_main)
    target_link_libraries(bench_main PRIVATE tos_core ubench tos_dynamic_log)
    set_target_properties(bench_main PROPERTIES CXX_EXTENSIONS OFF)

    if (TARGET arch_drivers)
        target_link_libraries(bench_main PRIVATE arch_drivers)
    endif()
endif()

function(add_benchmark target)
    message(STATUS "Benchmarking ${target}")
if (NOT TOS_BARE_LINKER)
    target_link_libraries(bench_main PRIVATE "-Wl,--whole-archive" ${target} "-Wl,--no-whole-archive")
else()
    target_link_libraries(bench_main PRIVATE "--whole-archive" ${target} "--no-whole-archive")
endif()
endfunction()

if (${TOS_PLATFORM} MATCHES hosted)
    target_sources(bench_main PRIVATE ${CMAKE_CURRENT_LIST_DIR}/hosted_bench_main.cpp)
elseif(${TOS_PLATFORM} MATCHES cc32xx)
    target_sources(bench_main PRIVATE ${CMAKE_CURRENT_LIST_DIR}/cc32xx_bench_main.cpp)
elseif(${TOS_PLATFORM} MATCHES esp)
    target_sources(bench_main PRIVATE ${CMAKE_CURRENT_LIST_DIR}/esp_bench_main.cpp)
elseif(${TOS_PLATFORM} MATCHES nrf52)
    target_sources(bench_main PRIVATE ${CMAKE_CURRENT_LIST_DIR}/nrf52_bench_main.cpp)
    target_link_libraries(bench_main PRIVATE tos_board)
else()
    target_sources(bench_main PRIVATE ${CMAKE_CURRENT_LIST_DIR}/stub_bench_main.cpp)
endif ()