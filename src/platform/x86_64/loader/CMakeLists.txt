project(tos C CXX ASM)

add_library(i386-loader-lib multiboot_boot.S)
target_link_libraries(i386-loader-lib PUBLIC "-T${CMAKE_CURRENT_SOURCE_DIR}/linker.ld")
target_link_libraries(i386-loader-lib PUBLIC tos::libcxx tos_util_core arch_i386 tos_print tos_elf)
target_include_directories(i386-loader-lib PUBLIC ../include)

set(LOADER_DIR ${CMAKE_CURRENT_SOURCE_DIR} CACHE STRING "")

include(CrossCmake)
function(add_i386_loader target source_dir source_target)
    add_executable(${target} ${LOADER_DIR}/core.cpp)
    target_link_libraries(${target} PUBLIC i386-loader-lib)

    build_other_target(${target} ${source_dir} ${source_target})

    include(FileEmbed)
    embed_file(${target} program_span ${source_dir}/bin/${source_target}.stripped)
endfunction()
