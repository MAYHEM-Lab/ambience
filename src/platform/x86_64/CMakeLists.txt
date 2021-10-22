project(tos C CXX ASM)

add_library(x86_64_core core.cpp include/tos/platform.hpp)
target_link_libraries(x86_64_core PUBLIC arch_x86_64)
target_include_directories(x86_64_core PUBLIC include)

add_library(x86_64_impl impl.cpp exceptions.S irq.cpp)
target_link_libraries(x86_64_impl PUBLIC x86_64_core tos_jobs tos::libcxx tos_util_core tos_print)
target_compile_options(x86_64_impl PRIVATE -mno-mmx -mno-sse -mno-sse2)

if (X86_64_LOADER MATCHES "limine")
    target_sources(x86_64_impl PRIVATE stivale2.h limine_boot.cpp)
    target_link_libraries(x86_64_impl PUBLIC "-T${CMAKE_CURRENT_SOURCE_DIR}/limine.ld")
elseif (X86_64_LOADER MATCHES "firecracker")
    target_sources(x86_64_impl PRIVATE firecracker_boot.cpp)
    target_link_libraries(x86_64_impl PUBLIC "-T${CMAKE_CURRENT_SOURCE_DIR}/firecracker.ld")
endif()