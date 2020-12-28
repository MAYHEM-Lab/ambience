project(tos C CXX ASM)

add_executable(i386-loader multiboot_boot.S core.cpp)
target_link_libraries(i386-loader PUBLIC "-T${CMAKE_CURRENT_SOURCE_DIR}/linker.ld")
target_link_libraries(i386-loader PUBLIC tos::libcxx tos_util_core arch_i386)
target_include_directories(i386-loader PUBLIC ../include)
