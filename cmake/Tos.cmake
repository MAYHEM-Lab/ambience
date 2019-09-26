function(print_size target)
    add_custom_command(
            TARGET ${target} POST_BUILD
            COMMAND ${CMAKE_SIZE} -A $<TARGET_FILE:${target}>
            COMMENT "Print size"
    )
endfunction()

function(get_implicit_flags FLAGS)
execute_process(
	COMMAND cmd.exe /c "${CMAKE_CXX_COMPILER} ${CMAKE_CXX_FLAGS} -xc++ -E -v -"
	TIMEOUT 1
	ERROR_VARIABLE FOO
	RESULT_VARIABLE RES
)
string(REGEX MATCH "cc1plus\.exe (.*)\\n" _ ${FOO})
set(${FLAGS} ${CMAKE_MATCH_1} PARENT_SCOPE)
endfunction()

set(TOS_FLAGS "-Wall -Wextra -Wpedantic \
     -ffunction-sections -fdata-sections -ffreestanding -g -pedantic -freorder-functions \
        -Wno-unknown-pragmas")

set(TOS_LINKER_FLAGS "-fno-threadsafe-statics -freorder-functions -fno-exceptions -fno-rtti -fno-unwind-tables")

set(TOS_FLAGS "${TOS_FLAGS} -fstack-usage")
set(TOS_LINKER_FLAGS "${TOS_LINKER_FLAGS} -Wl,--gc-sections -Xlinker -Map=output.map")

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    message(STATUS "Using gcc")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set(TOS_LINKER_FLAGS "${TOS_LINKER_FLAGS} -Wl,-dead_strip")
endif()

set(TOS_C_FLAGS "${TOS_FLAGS} -U__STRICT_ANSI__")
set(TOS_CXX_FLAGS "${TOS_FLAGS} -Wnon-virtual-dtor -fno-rtti -fno-exceptions \
    -fno-unwind-tables -fno-threadsafe-statics -Werror=return-type -std=c++14")

set(TOS ON)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${TOS_C_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${TOS_CXX_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${TOS_LINKER_FLAGS}")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS "${CMAKE_EXPORT_COMPILE_COMMANDS}" CACHE STRING "CMAKE_EXPORT_COMPILE_COMMANDS")

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

MACRO(SUBDIRLIST result curdir)
    FILE(GLOB children RELATIVE ${curdir} ${curdir}/*)
    SET(dirlist "")
    FOREACH(child ${children})
        IF(IS_DIRECTORY ${curdir}/${child})
            LIST(APPEND dirlist ${child})
        ENDIF()
    ENDFOREACH()
    SET(${result} ${dirlist})
ENDMACRO()