find_program(LIDLC_BIN
        NAMES lidlc
        HINTS /Users/fatih/lidl/cmake-build-debug/src/tools 
              /home/fatih/lidl/cmake-build-debug-llvm/src/tools
              /home/fatih/lidl/cmake-build-debug/src/tools
              /ambience/bin 
              ${CMAKE_BINARY_DIR}/bin 
              ${CMAKE_BINARY_DIR}/src/tools 
              ${CMAKE_BINARY_DIR}/src/tools/Debug
)

find_program(CLANG_FORMAT_BIN
        NAMES clang-format-8)

if (NOT ${LIDLC_BIN} MATCHES "NOTFOUND")
    message(STATUS "Found lidlc: ${LIDLC_BIN}")
endif()
if (NOT ${CLANG_FORMAT_BIN} MATCHES "NOTFOUND")
    message(STATUS "Found clang-format")
endif()

if (NOT ${LIDLC_BIN} MATCHES "NOTFOUND")
    function(add_lidlc Name)
        set(LIDLC_OUTPUTS)
        file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

        foreach(FILE ${ARGN})
            get_filename_component(LIDLC_OUTPUT ${FILE} NAME_WE)
            set(LIDLC_OUTPUT
                    "${CMAKE_CURRENT_BINARY_DIR}/${LIDLC_OUTPUT}_generated.hpp")
            list(APPEND LIDLC_OUTPUTS ${LIDLC_OUTPUT})
            SET_SOURCE_FILES_PROPERTIES(${LIDLC_OUTPUT} PROPERTIES GENERATED 1)

            set(INCLUDES -I$<JOIN:$<TARGET_PROPERTY:${Name},INTERFACE_INCLUDE_DIRECTORIES>,$<SEMICOLON>-I>)
            add_custom_command(OUTPUT ${LIDLC_OUTPUT}
                    COMMAND ${LIDLC_BIN}
                    ARGS -gcpp -f ${FILE} -o ${LIDLC_OUTPUT} "${INCLUDES}"
                    DEPENDS ${FILE} ${LIDLC_BIN}
                    COMMENT "Building C++ header for ${FILE}"
                    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                    COMMAND_EXPAND_LISTS)

            if (NOT ${CLANG_FORMAT_BIN} MATCHES "NOTFOUND")
                #[[add_custom_command(OUTPUT ${LIDLC_OUTPUT}
                        COMMAND ${CLANG_FORMAT_BIN}
                        ARGS -i ${LIDLC_OUTPUT}
                        DEPENDS ${LIDLC_OUTPUT} ${CLANG_FORMAT_BIN}
                        COMMENT "Formatting C++ header for ${FILE}"
                        WOLIDLC_BINRKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})]]
            endif()
        endforeach()
        add_custom_target(${Name}_IMPL DEPENDS ${LIDLC_OUTPUTS})

        add_library(${Name} INTERFACE)
        target_include_directories(${Name} INTERFACE ${CMAKE_CURRENT_BINARY_DIR})
        target_link_libraries(${Name} INTERFACE lidl_rt)
        add_dependencies(${Name} ${Name}_IMPL)
    endfunction()
else()
    function(add_lidlc Name)
        message(STATUS "lidlc not found!")
    endfunction()
endif()
