project(tos C CXX ASM)

function(add_i386_loader target source)
    add_executable(${target} multiboot_boot.S core.cpp)
    target_link_libraries(${target} PUBLIC "-T${CMAKE_CURRENT_SOURCE_DIR}/linker.ld")
    target_link_libraries(${target} PUBLIC tos::libcxx tos_util_core arch_i386 tos_print tos_elf)
    target_include_directories(${target} PUBLIC ../include)

    include(FileEmbed)
    embed_file(${target} program_span ${source})
endfunction()

include(AddIso)

add_i386_loader(i386-loader ${CMAKE_BINARY_DIR}/../cmake-build-barex64/bin/x64boot.stripped)
add_iso(i386-iso i386-loader)