function(executable_postbuild target)
    add_custom_command(
            TARGET ${target} POST_BUILD
            COMMAND ${CMAKE_OBJCOPY} $<TARGET_FILE:${target}> -O binary $<TARGET_FILE:${target}>.bin
            COMMENT "Convert to BIN image"
    )

    add_custom_command(
            TARGET ${target} POST_BUILD
            COMMAND ${CMAKE_OBJCOPY} $<TARGET_FILE:${target}> -O ihex $<TARGET_FILE:${target}>.hex
            COMMENT "Convert to Intel HEX image"
    )

if (CMAKE_SIZE)
    add_custom_command(
            TARGET ${target} POST_BUILD
            COMMAND ${CMAKE_SIZE} $<TARGET_FILE:${target}>
            COMMENT "Calculate size"
    )
endif()
endfunction()

function(add_executable target)
    _add_executable(${target} ${ARGN})
    get_target_property(IS_IMPORTED ${target} IMPORTED)
    if (NOT ${IS_IMPORTED})
        executable_postbuild(${target})

        if (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
            target_link_libraries(${target} PUBLIC "-Xlinker -Map=${CMAKE_BINARY_DIR}/maps/${target}.map")
        elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
            target_link_libraries(${target} PUBLIC "-Wl,-Map,${CMAKE_BINARY_DIR}/maps/${target}.map")
        endif()
    endif()
endfunction()

set(TOS_FLAGS "-Wall -Wextra -Wpedantic \
     -ffunction-sections -fdata-sections -ffreestanding -g -pedantic \
        -Wno-unknown-pragmas")

message(STATUS "${CMAKE_CXX_COMPILER_ID}")
if (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    message(STATUS "Using gcc")

    set(TOS_GCC_FLAGS "-fdump-ipa-cgraph -fstack-usage -freorder-functions")

    set(TOS_LINKER_FLAGS "-fno-threadsafe-statics -freorder-functions -fno-exceptions -fno-rtti -fno-unwind-tables")

    set(TOS_FLAGS "${TOS_FLAGS} ${TOS_GCC_FLAGS}")
    set(TOS_LINKER_FLAGS "${TOS_LINKER_FLAGS} -Wl,--gc-sections")
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    message(STATUS "Using Clang")
    set(TOS_FLAGS "${TOS_FLAGS} -D__ELF__")
endif()

set(TOS_C_FLAGS "${TOS_FLAGS} -U__STRICT_ANSI__")
set(TOS_CXX_FLAGS "${TOS_FLAGS} -Wnon-virtual-dtor -fno-rtti -fno-exceptions \
    -fno-unwind-tables -fno-threadsafe-statics -Werror=return-type")

set(TOS ON)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${TOS_C_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${TOS_CXX_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${TOS_LINKER_FLAGS}")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS "${CMAKE_EXPORT_COMPILE_COMMANDS}" CACHE STRING "CMAKE_EXPORT_COMPILE_COMMANDS")

file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/maps)

set(THIS_DIR ${CMAKE_CURRENT_LIST_DIR})
function(tos_install _target)
    set(INCLUDE_DEST "include")
    set(LIB_DEST "lib/${_target}")
    set(SHARE_DEST "share/${_target}")

    if (${ARGC} GREATER 1)
        set(HEADER_PATH ${ARGV1})
        #install(DIRECTORY ${HEADER_PATH}/ DESTINATION "${INCLUDE_DEST}")
    endif()

    target_include_directories(${_target} PUBLIC
            $<BUILD_INTERFACE:${HEADER_PATH}>
            $<INSTALL_INTERFACE:${INCLUDE_DEST}>)

    #[[install(TARGETS ${_target} DESTINATION "${LIB_DEST}")

    configure_file("${THIS_DIR}/cmake-config.cmake.in" ${CMAKE_CURRENT_BINARY_DIR}/${_target}-config.cmake @ONLY)
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${_target}-config.cmake DESTINATION ${LIB_DEST})

    install(TARGETS ${_target} EXPORT ${_target} DESTINATION "${LIB_DEST}")
    install(EXPORT ${_target} DESTINATION "${LIB_DEST}")

    if (${ARGC} GREATER 2)
        message(STATUS "Have a share dir, install it")
        set(SHARE_PATH ${ARGV2})
        install(DIRECTORY ${SHARE_PATH} DESTINATION "${SHARE_DEST}")
    endif()]]
endfunction()

install(FILES ${THIS_DIR}/tos-config.cmake DESTINATION "lib/tos")

include(TosFunctions)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}" CACHE STRING "CFLAGS")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}" CACHE STRING "CXXFLAGS")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS}" CACHE STRING "LINKERFLAGS")
