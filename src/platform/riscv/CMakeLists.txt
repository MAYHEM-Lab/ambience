project(tos C CXX ASM)

add_library(riscv_core include/tos/platform.hpp core.cpp)
target_link_libraries(riscv_core PUBLIC arch_x86_64)
target_include_directories(riscv_core PUBLIC include)

add_library(riscv_impl impl.cpp)
target_link_libraries(riscv_impl PUBLIC "-T${CMAKE_CURRENT_SOURCE_DIR}/linker.ld")
target_link_libraries(riscv_impl PUBLIC riscv_core tos_jobs tos::libcxx tos_util_core tos_print)
target_compile_options(riscv_impl PRIVATE -mno-mmx -mno-sse -mno-sse2)