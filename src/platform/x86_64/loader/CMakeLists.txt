project(tos C CXX ASM)

add_executable(i386-loader multiboot_boot.S core.cpp program.cpp)
target_link_libraries(i386-loader PUBLIC "-T${CMAKE_CURRENT_SOURCE_DIR}/linker.ld")
target_link_libraries(i386-loader PUBLIC tos::libcxx tos_util_core arch_i386 tos_print)
target_include_directories(i386-loader PUBLIC ../include)

find_package(Python3 REQUIRED COMPONENTS Interpreter)

add_custom_command(
    COMMAND Python3::Interpreter "${CMAKE_CURRENT_SOURCE_DIR}/bin2array.py" "${CMAKE_BINARY_DIR}/../cmake-build-bare-x64/bin/x64boot.bin" -O ${CMAKE_CURRENT_BINARY_DIR}/program.data
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/program.data
    DEPENDS ${CMAKE_BINARY_DIR}/../cmake-build-bare-x64/bin/x64boot.bin
)

target_include_directories(i386-loader PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

add_custom_target(i386-loader-program
        DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/program.data)

add_dependencies(i386-loader i386-loader-program)