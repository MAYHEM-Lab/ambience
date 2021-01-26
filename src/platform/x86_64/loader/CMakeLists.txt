project(tos C CXX ASM)

add_executable(i386-loader multiboot_boot.S core.cpp program.cpp)
target_link_libraries(i386-loader PUBLIC "-T${CMAKE_CURRENT_SOURCE_DIR}/linker.ld")
target_link_libraries(i386-loader PUBLIC tos::libcxx tos_util_core arch_i386 tos_print)
target_include_directories(i386-loader PUBLIC ../include)

find_package(Python3 REQUIRED COMPONENTS Interpreter)

add_custom_command(
    COMMAND Python3::Interpreter "${CMAKE_CURRENT_SOURCE_DIR}/bin2array.py" "${CMAKE_BINARY_DIR}/../cmake-build-barex64/bin/x64boot.bin" -O ${CMAKE_CURRENT_BINARY_DIR}/program.data
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/program.data
    DEPENDS ${CMAKE_BINARY_DIR}/../cmake-build-barex64/bin/x64boot.bin
)

target_include_directories(i386-loader PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

add_custom_target(i386-loader-program
        DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/program.data)

find_program(MKRESCUE_BIN grub-mkrescue)
if (MKRESCUE_BIN MATCHES "NOTFOUND")
    message(WARNING "grub-mkrescue was not found, will not generate a bootable ISO")
else()
    message(STATUS "Found grub-mkrescue(${MKRESCUE_BIN}), will generate a bootable ISO")
    file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/iso/boot/grub)
    add_custom_command(
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/grub.cfg ${CMAKE_CURRENT_BINARY_DIR}/iso/boot/grub/grub.cfg
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/iso/boot/grub/grub.cfg
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/grub.cfg
    )
    add_custom_command(
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:i386-loader> ${CMAKE_CURRENT_BINARY_DIR}/iso/boot
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/iso/boot/i386-loader
        DEPENDS i386-loader
    )
    add_custom_command(
        COMMAND ${MKRESCUE_BIN} -o ${CMAKE_CURRENT_BINARY_DIR}/Tos.iso ${CMAKE_CURRENT_BINARY_DIR}/iso
        DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/iso/boot/i386-loader ${CMAKE_CURRENT_BINARY_DIR}/iso/boot/grub/grub.cfg
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/Tos.iso
    )

    add_custom_target(i386-iso
            DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/Tos.iso)

endif()

add_dependencies(i386-loader i386-loader-program)