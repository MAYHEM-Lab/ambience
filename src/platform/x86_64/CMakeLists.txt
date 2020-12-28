if (${TOS_ARCH} MATCHES "i386")
    add_subdirectory(loader)
    return()
endif()

add_library(x86_64_core core.cpp include/tos/platform.hpp)
target_link_libraries(x86_64_core PUBLIC arch_x86_64)
target_include_directories(x86_64_core PUBLIC include)

add_library(x86_64_impl impl.cpp)
target_link_libraries(x86_64_impl PUBLIC "-T${CMAKE_CURRENT_SOURCE_DIR}/linker.ld")
target_link_libraries(x86_64_impl PUBLIC x86_64_core tos_jobs tos::libcxx tos_util_core)
target_compile_options(x86_64_impl PRIVATE -mno-mmx -mno-sse -mno-sse2)